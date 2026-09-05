"""Framework regression tests; no shared_ptr solution is embedded.
框架回归测试；不包含 shared_ptr 解答。
"""

import argparse
import json
import os
from pathlib import Path
import re
import shutil
import sys
import tempfile
import unittest
from unittest.mock import patch
import zipfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "autograder"))
from autograder import (GraderError, Outcome, OUTPUT_LIMIT, execute, grade, positive_seconds,
                        select_cases, write_report)
from cases import CASES, REJECTED_PROGRAMS
from package import PUBLIC_FILES, package


class InventoryTests(unittest.TestCase):
    def test_registry_matches_every_source(self):
        self.assertEqual(sum(map(len, CASES.values())), 50)
        self.assertEqual(len(REJECTED_PROGRAMS), 11)
        for part, cases in CASES.items():
            source = (ROOT / "autograder" / "tests" / f"{part}.cpp").read_text(encoding="utf-8")
            registered = re.findall(r"^TEST\((\w+)\)", source, flags=re.MULTILINE)
            self.assertEqual(len(registered), len(set(registered)))
            self.assertEqual(set(registered), set(cases), part)

    def test_selection_and_denominator(self):
        self.assertEqual(len(select_cases(None, None)), 50)
        self.assertEqual(len(select_cases(["basics"], None)) * 2, 14)
        selected = select_cases(None, ["basics.typed_null", "ownership.copy_*"])
        self.assertEqual(len(selected), 8)

    def test_bad_selection_is_not_success(self):
        with self.assertRaises(GraderError):
            select_cases(None, ["typo"])
        with self.assertRaises(GraderError):
            select_cases(["basics"], ["ownership.*"])

    def test_timeout_validation(self):
        for value in ("0", "-1", "nan", "inf", "-inf", "text"):
            with self.subTest(value=value), self.assertRaises(argparse.ArgumentTypeError):
                positive_seconds(value)
        self.assertEqual(positive_seconds("0.25"), 0.25)


class ScoringTests(unittest.TestCase):
    def arguments(self):
        return argparse.Namespace(submission=ROOT, cxx=os.environ.get("CXX", "g++"),
                                  sanitize=False, compile_timeout=5, timeout=1, quiet=True)

    def test_mixed_results_earn_exact_partial_credit(self):
        selected = select_cases(None, ["basics.empty_default", "basics.typed_null"])
        outcomes = [Outcome(0, "", "") for _ in range(3)]
        outcomes += [Outcome(0, "\n".join(CASES["basics"]) + "\n", ""),
                     Outcome(0, "PASS empty_default\n", ""), Outcome(1, "", "failed")]
        with patch("autograder.execute", side_effect=outcomes):
            report = grade(self.arguments(), selected)
        self.assertEqual((report["score"], report["max_score"]), (2, 4))
        self.assertEqual((report["passed"], report["failed"]), (1, 1))
        self.assertEqual([t["score"] for t in report["tests"]], [2, 0])

    def test_missing_registered_case_is_an_error(self):
        outcomes = [Outcome(0, "", "") for _ in range(3)]
        outcomes.append(Outcome(0, "empty_default\n", ""))
        with patch("autograder.execute", side_effect=outcomes), self.assertRaises(GraderError):
            grade(self.arguments(), select_cases(["basics"], None))


class ProcessTests(unittest.TestCase):
    def test_argument_boundaries_and_unicode(self):
        result = execute([sys.executable, "-c", "import sys; print(sys.argv[1])", "a b 中文"], 5, ROOT)
        self.assertTrue(result.ok)
        self.assertEqual(result.stdout.strip(), "a b 中文")

    def test_nonzero_and_crash(self):
        result = execute([sys.executable, "-c", "raise SystemExit(7)"], 5, ROOT)
        self.assertEqual(result.returncode, 7)
        if os.name == "posix":
            result = execute([sys.executable, "-c", "import os; os.kill(os.getpid(), 9)"], 5, ROOT)
            self.assertLess(result.returncode, 0)

    def test_timeout(self):
        result = execute([sys.executable, "-c", "import time; time.sleep(10)"], 0.1, ROOT)
        self.assertTrue(result.timed_out)
        self.assertFalse(result.ok)

    def test_output_is_bounded_and_keeps_tail(self):
        result = execute([sys.executable, "-c", "print('x' * 150000); print('tail')"], 5, ROOT)
        self.assertTrue(result.ok)
        self.assertLess(len(result.stdout), OUTPUT_LIMIT + 100)
        self.assertIn("truncated", result.stdout)
        self.assertTrue(result.stdout.endswith("tail\n"))

    def test_report_replaces_stale_data(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "nested" / "成绩.json"
            write_report(path, {"score": 100})
            write_report(path, {"status": "infrastructure_error", "error": "错误"})
            report = json.loads(path.read_text(encoding="utf-8"))
            self.assertNotIn("score", report)
            self.assertEqual(report["error"], "错误")
            self.assertEqual([p.name for p in path.parent.iterdir()], ["成绩.json"])


class RunnerTests(unittest.TestCase):
    def run_fixture(self, *, header=None, main=None, options=()):
        with tempfile.TemporaryDirectory(prefix="grader-fixture-") as temporary:
            folder = Path(temporary) / "student sources 中文"
            folder.mkdir()
            for name, replacement in (("shared_ptr.h", header), ("main.cpp", main)):
                original = (ROOT / name).read_text(encoding="utf-8")
                (folder / name).write_text(original if replacement is None else replacement,
                                           encoding="utf-8")
            report = folder / "report.json"
            command = [sys.executable, ROOT / "autograder" / "autograder.py", "--quiet",
                       "--submission", folder, "--json", report, *options]
            ran = execute(command, 60, ROOT)
            self.assertFalse(ran.timed_out, ran.diagnostic())
            self.assertTrue(report.exists(), ran.diagnostic())
            return ran, json.loads(report.read_text(encoding="utf-8"))

    def test_starter_subset_and_paths_with_spaces(self):
        ran, report = self.run_fixture(options=["--part", "basics"])
        self.assertEqual(ran.returncode, 1)
        self.assertEqual((report["score"], report["max_score"]), (0, 14))
        self.assertTrue(all(t["status"] == "failed" for t in report["tests"]))
        self.assertTrue(all("STUDENT TODO" in t["output"] for t in report["tests"]))

    def test_exit_zero_without_completion_marker_fails(self):
        starter = (ROOT / "shared_ptr.h").read_text(encoding="utf-8")
        header = starter.replace("std::abort();", "std::exit(0);")
        self.assertNotEqual(header, starter)
        ran, report = self.run_fixture(header=header, options=["--case", "basics.empty_default"])
        self.assertEqual(ran.returncode, 1)
        self.assertEqual(report["score"], 0)
        self.assertIn("exit=0", report["tests"][0]["output"])

    def test_infinite_loop_is_a_failed_case(self):
        starter = (ROOT / "shared_ptr.h").read_text(encoding="utf-8")
        header = starter.replace("std::abort();", "for (;;) {}")
        ran, report = self.run_fixture(header=header, options=["--case", "basics.empty_default", "--timeout", "0.2"])
        self.assertEqual(ran.returncode, 1)
        self.assertEqual(report["tests"][0]["status"], "timeout")

    def test_bad_application_does_not_block_core_suite(self):
        ran, report = self.run_fixture(main="invalid C++ source\n", options=[
            "--case", "basics.empty_default", "--case", "application.empty"])
        self.assertEqual(ran.returncode, 1)
        self.assertEqual(report["max_score"], 4)
        self.assertEqual([t["status"] for t in report["tests"]], ["failed", "compile_error"])
        self.assertIn("STUDENT TODO", report["tests"][0]["output"])

    def test_bad_header_scores_zero(self):
        ran, report = self.run_fixture(header="invalid C++ header\n", options=["--case", "basics.empty_default"])
        self.assertEqual(ran.returncode, 1)
        self.assertEqual(report["tests"][0]["status"], "compile_error")

    def test_missing_compiler_is_infrastructure_error(self):
        ran, report = self.run_fixture(options=["--cxx", "/nonexistent/shared-ptr-compiler"])
        self.assertEqual(ran.returncode, 2)
        self.assertEqual(report["status"], "infrastructure_error")
        self.assertNotIn("score", report)

    def test_unknown_case_is_infrastructure_error(self):
        ran, report = self.run_fixture(options=["--case", "not_a_real_case"])
        self.assertEqual(ran.returncode, 2)
        self.assertEqual(report["status"], "infrastructure_error")


class PackagingTests(unittest.TestCase):
    def test_archive_allowlist_and_no_overwrite(self):
        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary) / "student.zip"
            package(ROOT, output)
            with zipfile.ZipFile(output) as archive:
                names = archive.namelist()
                self.assertEqual(set(names), {"shared-pointer-lab/" + p for p in PUBLIC_FILES})
                self.assertFalse(any("/instructor/" in n or "/build/" in n or "/.git/" in n for n in names))
                self.assertIn(b"STUDENT TODO", archive.read("shared-pointer-lab/shared_ptr.h"))
            before = output.read_bytes()
            with self.assertRaises(FileExistsError):
                package(ROOT, output)
            self.assertEqual(output.read_bytes(), before)

    def test_changed_starter_is_not_distributed(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary) / "source"
            for name in (*PUBLIC_FILES, "instructor/starter.sha256"):
                target = root / name
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(ROOT / name, target)
            (root / "shared_ptr.h").write_text("altered source\n", encoding="utf-8")
            output = Path(temporary) / "student.zip"
            with self.assertRaises(ValueError):
                package(root, output)
            self.assertFalse(output.exists())


class AllocationHarnessTests(unittest.TestCase):
    def test_allocation_accounting_and_failure_injection(self):
        # These probes test the harness itself, without implementing a smart pointer.
        # 以下探针只测试框架本身，不实现智能指针。
        source = '''#include "test.hpp"
#include <new>
int* escaped = nullptr;
TEST(clean) { auto* p = new int(4); CHECK(*p == 4); delete p; }
TEST(leak) { escaped = new int(4); CHECK(*escaped == 4); }
TEST(failure_once) {
    bool caught = false;
    lab_memory::FailAfter fail(0);
    try { escaped = new int(1); } catch (const std::bad_alloc&) { caught = true; }
    CHECK(caught);
    auto* p = new int(2); CHECK(*p == 2); delete p;
}
TEST(nothrow_failure) {
    lab_memory::FailAfter fail(0);
    auto* p = new (std::nothrow) int(1); CHECK(p == nullptr);
}
TEST(aligned_array) {
    struct alignas(128) Aligned { int n = 9; };
    auto* p = new Aligned[3]; CHECK(p[2].n == 9); delete[] p;
}
'''
        with tempfile.TemporaryDirectory(prefix="harness-probe-") as temporary:
            directory = Path(temporary)
            path = directory / "probe.cpp"
            path.write_text(source, encoding="utf-8")
            binary = directory / "probe"
            compiler = shutil.which(os.environ.get("CXX", "g++"))
            self.assertIsNotNone(compiler)
            compiled = execute([compiler, "-std=c++20", "-O0", "-I", ROOT / "autograder",
                                path, ROOT / "autograder" / "memory.cpp", "-o", binary], 60, ROOT)
            self.assertTrue(compiled.ok, compiled.diagnostic())
            for name in ("clean", "failure_once", "nothrow_failure", "aligned_array"):
                with self.subTest(name=name):
                    ran = execute([binary, name], 5, ROOT)
                    self.assertTrue(ran.ok, ran.diagnostic())
                    self.assertEqual(ran.stdout.strip(), f"PASS {name}")
            leak = execute([binary, "leak"], 5, ROOT)
            self.assertEqual(leak.returncode, 1)
            self.assertIn("Allocation imbalance: +1", leak.stderr)
            unknown = execute([binary, "unknown_case"], 5, ROOT)
            self.assertEqual(unknown.returncode, 2)


if __name__ == "__main__":
    unittest.main()
