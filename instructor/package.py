#!/usr/bin/env python3
"""Create an allowlisted student archive from the pristine starter.
从原始骨架按白名单生成学生包。
"""

import argparse
import hashlib
import os
from pathlib import Path
import sys
import tempfile
import zipfile

ROOT = Path(__file__).resolve().parents[1]
PUBLIC_FILES = (
    ".gitignore", "README.md", "Makefile", "shared_ptr.h", "main.cpp", "short_answer.txt",
    "docs/spec.md", "docs/grading.md", "docs/instructor.md",
    "autograder/autograder.py", "autograder/cases.py", "autograder/memory.cpp",
    "autograder/memory.hpp", "autograder/test.hpp",
    "autograder/tests/basics.cpp", "autograder/tests/ownership.cpp",
    "autograder/tests/modifiers.cpp", "autograder/tests/factory.cpp",
    "autograder/tests/application.cpp", "autograder/tests/exceptions.cpp",
    "autograder/tests/interface.cpp", "autograder/tests/other_tu.cpp",
)


def package(root, output):
    root, output = Path(root).resolve(), Path(output).absolute()
    if output.exists() or output.is_symlink():
        raise FileExistsError(f"Archive already exists / 压缩包已存在: {output}")
    for line in (root / "instructor/starter.sha256").read_text(encoding="utf-8").splitlines():
        expected, name = line.split(maxsplit=1)
        actual = hashlib.sha256((root / name).read_bytes()).hexdigest()
        if actual != expected:
            raise ValueError(f"Starter has changed; refusing to package possible answers / "
                             f"骨架已变化，拒绝打包可能的答案: {name}")
    payloads = []
    for name in PUBLIC_FILES:
        path = root / name
        if path.is_symlink() or not path.is_file():
            raise ValueError(f"Missing or linked public file / 公开文件缺失或为链接: {name}")
        payloads.append((name, path.read_bytes()))
    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(dir=output.parent, prefix=".student-", suffix=".zip", delete=False) as stream:
        temporary = Path(stream.name)
    try:
        with zipfile.ZipFile(temporary, "w", compression=zipfile.ZIP_DEFLATED) as archive:
            for name, data in payloads:
                archive.writestr("shared-pointer-lab/" + name, data)
        # Hard-link publication is atomic and refuses to overwrite an existing archive.
        # 硬链接发布是原子的，并拒绝覆盖已经存在的压缩包。
        os.link(temporary, output)
    finally:
        temporary.unlink(missing_ok=True)
    return output


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=ROOT / "dist/shared-pointer-lab.zip")
    args = parser.parse_args()
    try:
        output = package(ROOT, args.output)
    except (OSError, ValueError) as error:
        print(str(error), file=sys.stderr)
        return 1
    print(f"Student archive / 学生包: {output}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
