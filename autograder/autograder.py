#!/usr/bin/env python3
"""Compile and grade isolated cases using only the Python standard library.
仅使用 Python 标准库，编译并评分相互隔离的用例。
"""

import argparse
from dataclasses import dataclass
import fnmatch
import json
import math
import os
from pathlib import Path
import shutil
import signal
import subprocess
import sys
import tempfile

from cases import CASES, REJECTED_PROGRAMS

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
OUTPUT_LIMIT = 65536


class GraderError(Exception):
    """Configuration or infrastructure failure, not a student test result.
    配置或基础设施错误，不作为学生测试结果。
    """


@dataclass
class Outcome:
    returncode: int
    stdout: str
    stderr: str
    timed_out: bool = False

    @property
    def ok(self):
        return not self.timed_out and self.returncode == 0

    def diagnostic(self):
        status = "TIMEOUT / 超时" if self.timed_out else f"exit={self.returncode}"
        return f"{status}\n{self.stdout}{self.stderr}".strip()


def read_output(stream):
    stream.seek(0, os.SEEK_END)
    size = stream.tell()
    stream.seek(0)
    if size <= OUTPUT_LIMIT:
        data = stream.read()
    else:
        first = stream.read(OUTPUT_LIMIT // 2)
        stream.seek(-OUTPUT_LIMIT // 2, os.SEEK_END)
        data = first + b"\n[output truncated / output limit]\n" + stream.read()
    return data.decode("utf-8", errors="replace")


def execute(command, timeout, cwd, env=None):
    # Spool output to disk to avoid unbounded capture in the grader process.
    # 输出暂存到磁盘，避免评分器进程无限积累捕获内容。
    with tempfile.TemporaryFile() as out, tempfile.TemporaryFile() as err:
        try:
            child = subprocess.Popen(
                [str(item) for item in command], cwd=cwd, env=env,
                stdin=subprocess.DEVNULL, stdout=out, stderr=err,
                start_new_session=(os.name == "posix"),
            )
        except OSError as error:
            raise GraderError(f"Cannot execute / 无法执行 {command[0]}: {error}") from error
        timed_out = False
        try:
            child.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            timed_out = True
            if os.name == "posix":
                try:
                    os.killpg(child.pid, signal.SIGKILL)
                except ProcessLookupError:
                    pass
            else:
                child.kill()
            child.wait()
        return Outcome(child.returncode, read_output(out), read_output(err), timed_out)


def positive_seconds(value):
    try:
        number = float(value)
    except ValueError as error:
        raise argparse.ArgumentTypeError("Expected seconds / 请输入秒数") from error
    if not math.isfinite(number) or number <= 0:
        raise argparse.ArgumentTypeError("Timeout must be positive and finite / 超时须为有限正数")
    return number


def select_cases(parts, patterns):
    available = [(part, case, hint) for part, cases in CASES.items()
                 for case, hint in cases.items()]
    for pattern in patterns or []:
        if not any(fnmatch.fnmatchcase(f"{part}.{case}", pattern) for part, case, _ in available):
            raise GraderError(f"Unknown case pattern / 未匹配到用例: {pattern}")
    selected = [(part, case, hint) for part, case, hint in available
                if (not parts or part in parts)
                and (not patterns or any(fnmatch.fnmatchcase(f"{part}.{case}", pattern)
                                         for pattern in patterns))]
    if not selected:
        raise GraderError("Empty selection / 筛选结果为空")
    return selected


def compiler_flags(sanitize):
    flags = ["-std=c++20", "-Wall", "-Wextra", "-Wpedantic", "-g", "-O0"]
    if sanitize:
        flags += ["-fsanitize=address,undefined", "-fno-omit-frame-pointer",
                  "-fno-sanitize-recover=all"]
        if sys.platform.startswith("linux"):
            # Avoid ASan shadow-address collisions with PIE on some Linux hosts.
            # 避免部分 Linux 主机上 PIE 与 ASan 影子地址发生冲突。
            flags += ["-fno-pie", "-no-pie"]
    return flags


def run_environment(sanitize):
    env = os.environ.copy()
    if sanitize:
        env["ASAN_OPTIONS"] = "detect_leaks=1:halt_on_error=1:abort_on_error=0"
        env["UBSAN_OPTIONS"] = "halt_on_error=1:print_stacktrace=1"
        env["LSAN_OPTIONS"] = "exitcode=23"
    return env


def require_toolchain(compiler, flags, build, timeout, env):
    source = build / "toolchain.cpp"
    source.write_text("static_assert(__cplusplus >= 202002L);\nint main() { return 0; }\n",
                      encoding="utf-8")
    binary = build / "toolchain"
    compiled = execute([compiler, *flags, source, "-o", binary], timeout, build, env)
    if not compiled.ok:
        raise GraderError("C++20 toolchain probe failed / C++20 工具链探测失败\n" + compiled.diagnostic())
    ran = execute([binary], timeout, build, env)
    if not ran.ok:
        raise GraderError("Runtime/sanitizer probe failed / 运行环境或检测器探测失败\n" + ran.diagnostic())


def check_rejections(compiler, flags, stage, build, timeout, env):
    for name, body in REJECTED_PROGRAMS.items():
        source = build / f"reject_{name}.cpp"
        source.write_text('#include "shared_ptr.h"\nint main() { ' + body + ' }\n', encoding="utf-8")
        result = execute([compiler, *flags, "-I", stage, "-fsyntax-only", source], timeout, build, env)
        if result.timed_out or result.returncode < 0:
            return False, f"Compiler failed during / 编译器异常: {name}\n{result.diagnostic()}"
        if result.returncode == 0:
            return False, f"Invalid program was accepted / 错误代码被接受: {name}\n{body}"
        # A real compiler diagnostic is required; a silent exit is not a valid rejection.
        # 必须有真实编译诊断；静默退出不视为正确拒绝。
        if not result.stderr.strip():
            return False, f"Missing rejection diagnostic / 缺少拒绝诊断: {name}"
    return True, ""


def grade(args, selected):
    submission = args.submission.resolve()
    if not (submission / "shared_ptr.h").is_file():
        raise GraderError(f"Missing shared_ptr.h / 缺少提交头文件: {submission}")
    compiler = shutil.which(args.cxx)
    if compiler is None:
        raise GraderError(f"Compiler not found / 未找到编译器: {args.cxx}")
    flags = compiler_flags(args.sanitize)
    env = run_environment(args.sanitize)
    results = []
    with tempfile.TemporaryDirectory(prefix="shared-ptr-grade-") as temporary:
        build = Path(temporary)
        stage = build / "submission"
        stage.mkdir()
        # Only student source files are staged; the harness is always the trusted local copy.
        # 仅暂存学生源文件；测试框架始终使用当前可信副本。
        for name in ("shared_ptr.h", "main.cpp"):
            source = submission / name
            if source.is_file():
                shutil.copyfile(source, stage / name)
        require_toolchain(compiler, flags, build, args.compile_timeout, env)
        for part in dict.fromkeys(part for part, _, _ in selected):
            binary = build / part
            sources = [HERE / "tests" / f"{part}.cpp", HERE / "memory.cpp"]
            if part == "interface":
                sources.append(HERE / "tests" / "other_tu.cpp")
            command = [compiler, *flags, "-I", stage, "-I", HERE, *sources, "-o", binary]
            compiled = execute(command, args.compile_timeout, build, env)
            if compiled.ok:
                listing = execute([binary, "--list"], args.timeout, build, env)
                names = listing.stdout.splitlines()
                if not listing.ok or len(names) != len(set(names)) or set(names) != set(CASES[part]):
                    raise GraderError(f"Case registry mismatch / 用例注册不一致: {part}\n" + listing.diagnostic())
            for suite, case, hint in selected:
                if suite != part:
                    continue
                passed = False
                if not compiled.ok:
                    status = "compile_error"
                    output = compiled.diagnostic()
                else:
                    ran = execute([binary, case], args.timeout, build, env)
                    lines = ran.stdout.rstrip().splitlines()
                    passed = ran.ok and bool(lines) and lines[-1] == f"PASS {case}"
                    status = "passed" if passed else ("timeout" if ran.timed_out else "failed")
                    output = "" if passed else ran.diagnostic()
                    if passed and part == "interface" and case == "rejected_programs":
                        passed, output = check_rejections(compiler, flags, stage, build,
                                                          args.compile_timeout, env)
                        status = "passed" if passed else "failed"
                record = {"name": f"{part}.{case}", "part": part, "status": status,
                          "score": 2 if passed else 0, "max_score": 2, "hint": hint, "output": output}
                results.append(record)
                if not args.quiet:
                    print(f"{'PASS' if passed else 'FAIL'} {record['name']}  {record['score']}/2", flush=True)
                    if not passed:
                        print(f"  {hint}\n{output[:4000]}", flush=True)
    return {"schema_version": 1, "status": "completed", "submission": str(submission),
            "compiler": compiler, "sanitized": args.sanitize,
            "score": sum(r["score"] for r in results), "max_score": 2 * len(results),
            "passed": sum(r["status"] == "passed" for r in results),
            "failed": sum(r["status"] != "passed" for r in results), "tests": results}


def write_report(path, report):
    path = path.resolve()
    path.parent.mkdir(parents=True, exist_ok=True)
    # Replace the requested report atomically, never leave a partially written grade.
    # 原子替换指定报告，避免留下部分写入的成绩。
    with tempfile.NamedTemporaryFile(mode="w", encoding="utf-8", dir=path.parent,
                                     prefix=".grade-", delete=False) as stream:
        temporary = Path(stream.name)
        try:
            json.dump(report, stream, ensure_ascii=False, indent=2)
            stream.write("\n")
        except BaseException:
            temporary.unlink(missing_ok=True)
            raise
    try:
        os.replace(temporary, path)
    finally:
        temporary.unlink(missing_ok=True)


def main(argv=None):
    parser = argparse.ArgumentParser(description="Shared Pointer Lab autograder / 自动评分")
    parser.add_argument("--submission", type=Path, default=ROOT, help="Student source directory / 学生源码目录")
    parser.add_argument("--cxx", default=os.environ.get("CXX", "g++"), help="Compiler executable / 编译器可执行文件")
    parser.add_argument("--part", action="append", choices=CASES, help="Select a suite / 选择测试组")
    parser.add_argument("--case", action="append", help="Select case ID or glob / 用例名或通配符")
    parser.add_argument("--list", action="store_true", help="List selected cases / 列出所选用例")
    parser.add_argument("--sanitize", action="store_true", help="Enable ASan/UBSan/LSan / 启用内存及未定义行为检查")
    parser.add_argument("--json", type=Path, help="Write JSON report / 写入 JSON 成绩")
    parser.add_argument("--timeout", type=positive_seconds, default=5.0, help="Seconds per case / 单用例超时秒数")
    parser.add_argument("--compile-timeout", type=positive_seconds, default=60.0,
                        help="Seconds per compilation / 单次编译超时秒数")
    parser.add_argument("--quiet", action="store_true", help="Only print final totals / 只打印最终统计")
    args = parser.parse_args(argv)
    try:
        selected = select_cases(args.part, args.case)
        if args.list:
            for part, case, hint in selected:
                print(f"{part}.{case}\t2\t{hint}")
            return 0
        report = grade(args, selected)
        code = 0 if report["failed"] == 0 else 1
        print(f"Score / 得分: {report['score']}/{report['max_score']}  "
              f"Passed / 通过: {report['passed']}/{len(report['tests'])}")
    except (GraderError, OSError) as error:
        report = {"schema_version": 1, "status": "infrastructure_error", "error": str(error)}
        print(f"ERROR / 错误: {error}", file=sys.stderr)
        code = 2
    if args.json:
        try:
            write_report(args.json, report)
        except OSError as error:
            print(f"Cannot write report / 无法写入报告: {error}", file=sys.stderr)
            return 2
    return code


if __name__ == "__main__":
    sys.exit(main())
