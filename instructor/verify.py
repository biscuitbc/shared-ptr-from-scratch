#!/usr/bin/env python3
"""Verify the starter and optionally an external, private complete solution.
验证学生骨架，并可选验证仓库外的私有完整实现。
"""

import argparse
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "autograder"))
from autograder import execute


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def grade_source(source, cxx, sanitized, output):
    command = [sys.executable, ROOT / "autograder" / "autograder.py", "--submission", source,
               "--cxx", cxx, "--quiet", "--json", output]
    if sanitized:
        command.append("--sanitize")
    result = subprocess.run([str(item) for item in command], cwd=ROOT, check=False)
    require(output.is_file(), "Missing report / 缺少评分报告")
    report = json.loads(output.read_text(encoding="utf-8"))
    require(report["status"] == "completed", str(report))
    return result.returncode, report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--solution", type=Path, help="External private solution directory / 仓库外私有解答目录")
    parser.add_argument("--cxx", default=os.environ.get("CXX", "g++"))
    args = parser.parse_args()
    env = os.environ.copy()
    env["CXX"] = args.cxx
    tested = subprocess.run([sys.executable, "-m", "unittest", "discover", "-s", "instructor",
                             "-p", "test_grader.py", "-v"], cwd=ROOT, env=env, check=False)
    require(tested.returncode == 0, "Framework regression failed / 框架回归失败")
    with tempfile.TemporaryDirectory(prefix="shared-ptr-verify-") as temporary:
        directory = Path(temporary)
        code, report = grade_source(ROOT, args.cxx, False, directory / "starter.json")
        require(code == 1 and report["score"] == 0 and report["max_score"] == 100,
                "Starter must score 0/100 / 骨架应得 0/100")
        require(all(t["status"] == "failed" and "STUDENT TODO" in t["output"] for t in report["tests"]),
                "Every starter case must compile and reach a TODO / 骨架每项必须编译成功并报告 TODO")
        demo = directory / "demo"
        compiled = execute([args.cxx, "-std=c++20", ROOT / "main.cpp", "-o", demo], 60, ROOT)
        require(compiled.ok, compiled.diagnostic())
        if args.solution:
            solution = args.solution.resolve()
            require(ROOT not in solution.parents and solution != ROOT,
                    "Keep complete solutions outside the repository / 完整解答须放在仓库外")
            for sanitized in (False, True):
                code, report = grade_source(solution, args.cxx, sanitized, directory / "solution.json")
                require(code == 0 and report["score"] == 100 and report["passed"] == 50,
                        "Private solution failed / 私有实现验证失败: " + str(report))
            compiled = execute([args.cxx, "-std=c++20", solution / "main.cpp", "-o", demo], 60, ROOT)
            require(compiled.ok, compiled.diagnostic())
            ran = execute([demo], 5, ROOT)
            require(ran.ok and ran.stdout == "1 -> 2 -> 7 -> 8\n3 -> 7 -> 8\ntail owners: 3\n",
                    "Demo output mismatch / 演示输出不匹配: " + ran.diagnostic())
    print("Framework verification passed / 框架验证通过")


if __name__ == "__main__":
    try:
        main()
    except (RuntimeError, OSError) as error:
        print(str(error), file=sys.stderr)
        sys.exit(1)
