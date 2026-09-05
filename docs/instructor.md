# Instructor guide / 教师说明

## English

The repository contains the assignment, public tests, and verification tools. It does not ship a completed `shared_ptr` or `create_list`. Keep any complete solution in a private directory outside the repository, including during verification.

### Verify the framework

```bash
python3 instructor/verify.py
python3 instructor/verify.py --solution /absolute/private-solution
```

The first command checks the grader itself, ensures all 50 starter cases compile and reach a TODO in normal and sanitizer modes, verifies the expected initial 0/100, and compiles the starter demo. Run it against the unmodified starter. Student implementations should use the regular grader instead. GitHub Actions runs these checks and packaging on both GCC and Clang for pushes and pull requests; CI never needs a complete solution.

The optional private directory contains only the instructor's `shared_ptr.h` and `main.cpp`. The second command additionally requires that implementation to score 100/100 both normally and with sanitizers, then checks the standalone demo's exact output. The tool rejects a solution directory within this repository. Private code is only copied into a temporary grading directory and is never committed, archived, or uploaded by these tools.

The Python regression suite checks inventory consistency, filtering and denominators, invalid timeouts, Unicode/spaced paths, process signals and timeouts, bounded diagnostic capture, stale-report replacement, silent early exit, compile-failure isolation, infrastructure errors, allocation accounting and failure injection.

```bash
python3 -m unittest discover -s instructor -p 'test_grader.py' -v
```

Also validate that plausible incorrect implementations fail: missing copy increments, uncleared moved-from state, leaked object/control block, incorrect typed-null counts, pointer-only swaps, destructive self-move, early release in member-source assignment, lost reset exception guarantees, missing allocation-failure cleanup, broken forwarding, and copied/dropped list tails. Prepare such variants from an external private solution; do not embed answers or solution patches in the public repository. A positive reference run alone is insufficient evidence that a grader catches these bugs.

### Student distribution

```bash
python3 instructor/package.py --output dist/shared-pointer-lab.zip
```

The package uses an explicit allowlist, includes the starter, docs, Makefile and public tests, and excludes `.git`, build output, reports, private solutions and instructor tools. It verifies the three student files against `instructor/starter.sha256` and refuses to package modified files or overwrite an existing archive. After intentionally revising a pristine starter, review the TODOs and update these fingerprints with `sha256sum shared_ptr.h main.cpp short_answer.txt`. Inspect the archive before distribution. The source repository's `make verify` target is a maintainer command and is not included as a usable workflow in the student archive.

### Manual review

The automatic score is 100 points; short answers are separate. Check that students implement their own object/control-block ownership and do not delegate it to library smart pointers. Preserve provided public signatures and helpers. Read the eight answers for understanding of shared counts, constness, copy/move, member-source assignment, exceptions, structural sharing, cycles and recursive destruction.

The [contract](spec.md) is authoritative for teaching-specific choices: separate object/block allocation, no arrays or weak pointers, and self-move preservation. Do not add hidden requirements beyond that contract. Test additions should include an expected-pass private implementation and an expected-fail variant where practical.

### Provenance

The staged implementation/application/short-answer format was inspired by [CS106L Assignment 7](https://github.com/cs106l/cs106l-assignments/tree/main/assignment7). The implementation, tests and bilingual documentation here are independently written, use namespace `lab`, and are not an official course assignment. Smart-pointer behavioral sources are linked in the contract.

---

## 中文

仓库仅提供实验、公开测试和验证工具，不附带完整 `shared_ptr` 或 `create_list` 解答。包括验证期间在内，完整实现都应保存在仓库外的私有目录。

### 验证框架

```bash
python3 instructor/verify.py
python3 instructor/verify.py --solution /absolute/private-solution
```

第一条命令检查评分器自身，确认初始 50 项在普通和 sanitizer 模式下均可编译且执行到 TODO，核对初始得分 0/100，并编译学生演示程序。它用于未修改的骨架，学生完成后的代码应使用普通评分器。GitHub Actions 会在 push 和 pull request 时使用 GCC、Clang 执行这些检查及打包；CI 不需要完整解答。

私有目录放置教师的 `shared_ptr.h` 和 `main.cpp`。第二条命令额外要求私有实现的普通模式与 sanitizer 模式都得 100/100，再检查独立演示程序的精确输出。工具拒绝使用仓库内部的解答目录。私有代码只复制到临时评分目录，这些工具不会提交、打包或上传它。

Python 回归测试涵盖清单一致性、筛选和分母、非法超时、中文及带空格路径、进程信号和超时、诊断长度限制、旧报告替换、提前静默退出、编译错误隔离、基础设施错误、分配余额和分配失败注入。

```bash
python3 -m unittest discover -s instructor -p 'test_grader.py' -v
```

还应验证常见错误实现会被拒绝：复制漏加计数、移动后源未清空、对象/控制块泄漏、有类型空指针计数错误、只交换对象指针、自移动破坏状态、成员源赋值时过早释放、reset 丧失异常保证、分配失败漏清理、转发错误、复制或丢弃链表尾部。应从仓库外的私有实现制作这些变体，不要把答案或答案补丁写入公开仓库。仅有正确参考解通过，不足以证明评分器能捕获这些错误。

### 发布学生包

```bash
python3 instructor/package.py --output dist/shared-pointer-lab.zip
```

打包使用明确白名单，包含骨架、文档、Makefile 和公开测试，排除 `.git`、构建结果、报告、私有解答和教师工具。它会用 `instructor/starter.sha256` 核对三个学生文件，拒绝打包已修改文件或覆盖已有压缩包。维护者有意修改原始骨架后，应检查 TODO，并使用 `sha256sum shared_ptr.h main.cpp short_answer.txt` 更新指纹。发布前检查压缩包。源仓库中的 `make verify` 是维护命令，学生包不提供该工作流。

### 人工检查

自动评分为 100 分，简答题单独评价。检查学生是否自己管理对象和控制块，而不是委托给标准库智能指针；是否保留公开签名和已提供的辅助代码。八道问题用于检查对共享计数、const、复制/移动、成员源赋值、异常、结构共享、环和递归析构的理解。

[接口契约](spec.md)规定实验选择：对象/控制块分别分配、不支持数组及弱引用、自移动保留原值。不要添加契约之外的隐藏要求。增加测试时，条件允许则同时准备应通过的私有实现和应失败的变体。

### 参考来源

“实现、应用、简答题”的阶段结构参考 [CS106L Assignment 7](https://github.com/cs106l/cs106l-assignments/tree/main/assignment7)。本仓库代码、测试和双语文档独立编写，使用 `lab` 命名空间，不是官方课程作业。智能指针语义的参考来源见接口契约。
