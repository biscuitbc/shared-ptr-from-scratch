# Shared Pointer Lab

## English

This lab is inspired by [Stanford CS106L Assignment 7](https://github.com/cs106l/cs106l-assignments/tree/main/assignment7), following its staged format of implementation, application, and short answers. It adapts the original `unique_ptr` exercise to shared ownership: build a small C++20 `lab::shared_ptr<T>` from scratch, then use it to build linked lists with shared tails.

### Learning goals

Practice RAII, templates, the Rule of Five, operator overloading, reference counting, exception safety, and perfect forwarding. Prerequisites: classes, pointers, `new/delete`, templates, lvalue/rvalue references, and basic exceptions.

### Quick start

Supported platform: Linux / WSL. Requirements: a C++20 GCC or Clang compiler and Python 3.9+. No third-party Python packages or downloaded test framework are needed.

```bash
g++ --version
python3 --version
python3 autograder/autograder.py
```

The starter compiles. An unfinished function prints `STUDENT TODO` and aborts. The expected initial grade is **0/100 with exit code 1**. Run focused tests as you implement each stage:

```bash
python3 autograder/autograder.py --list
python3 autograder/autograder.py --part basics
python3 autograder/autograder.py --case ownership.copy_constructor
python3 autograder/autograder.py --sanitize --json build/results.json
```

The grader rebuilds from source on every run. Each case executes in a separate process. A failure or crash does not stop the remaining cases. Filtering with `--part` or `--case` adjusts the score denominator. The complete suite has 50 cases worth 2 points each, totaling 100. Short answers are reviewed separately. See [grading](docs/grading.md) for details.

### Student files

| File | Task |
| --- | --- |
| `shared_ptr.h` | Implement all `STUDENT TODO` tasks S1–S7. Private fields and helpers may be added. |
| `main.cpp` | Implement S8, `create_list`. |
| `short_answer.txt` | Answer eight questions in your own words, once per question in either language. |

Preserve the public signatures and provided helpers. Do not edit the tests to earn points. `autograder/` contains public tests; `docs/` contains instructions; `instructor/` contains framework checks only. Complete solutions are not included. Instructors may supply an external private solution for verification.

### Part 1: Implement shared ownership

Read the [interface contract](docs/spec.md) before coding. The lab supports single-threaded, single-object, same-type ownership; it does not implement the entire standard library API.

1. **S1 — State and observers.** Design the stored pointer and shared count. Implement empty constructors, `get`, dereference, arrow, `use_count`, and explicit boolean conversion.
2. **S2 — Adopt a raw pointer.** Create a control block with count 1. Clean up the object if control-block allocation fails.
3. **S3 — RAII.** Release one share on destruction. Only the last owner deletes the object and control block. Complete S1–S3 before expecting all `basics` cases to pass.
4. **S4 — Copy.** Share ownership without copying `T`. Assignment must handle previous ownership, self-assignment, same-group assignment, and `p = p->next`.
5. **S5 — Move.** Transfer state and leave the source default-empty. Preserve self-moves; distinct handles in the same group still require a real transfer.
6. **S6 — Modifiers.** Implement `reset` and `swap`. `reset(T*)` preserves old ownership if allocation fails.
7. **S7 — Factory.** Combine `new T(std::forward<Args>(args)...)` with the raw-pointer constructor. Support zero arguments and noncopyable arguments.

Each stub states its task ID and postconditions. Remove its `detail::todo(...)` call when implemented. The stubs abort because throwing an exception inside a `noexcept` function would also terminate execution.

Implement ownership yourself: do not delegate it to standard or third-party smart pointers. Ordinary utilities such as `std::swap`, `std::exchange`, and type traits are allowed. Use scalar `new/delete`; arrays are out of scope.

Maintain one shared counter per ownership group, with one share contributed by each owning handle. A null stored pointer can still own a control block: literal `nullptr` produces count 0, while `static_cast<T*>(nullptr)` passed to the raw-pointer constructor produces count 1. `const shared_ptr<T>` does not make `T` const. See the contract for these boundaries and exception guarantees.

### Part 2: Share a linked-list tail

Implement `create_list(values, tail)` in `main.cpp`. Allocate a fresh prefix in the order of `values`, then share the original `tail` without copying its nodes. Empty values return `tail`. Reverse iterators avoid unsigned-index underflow.

```text
first  --> [1] --> [2] --+
                        +--> [7] --> [8]
second --> [3] ----------+
tail   -----------------+
```

Mutating a shared node is visible to all its users. Releasing `first` frees nodes 1 and 2; other handles keep nodes 7 and 8 alive. `tail.use_count()` counts handles directly owning node 7's control block, not reachable paths.

```bash
g++ -std=c++20 -Wall -Wextra -Wpedantic main.cpp -o main
./main
```

Expected output after implementation:

```text
1 -> 2 -> 7 -> 8
3 -> 7 -> 8
tail owners: 3
```

Do not create cycles. Member destruction reclaims the list recursively; very long lists can exhaust the call stack. Tests use moderate lengths.

### Before submission

```bash
python3 autograder/autograder.py
python3 autograder/autograder.py --sanitize
```

Pass both runs and complete the short answers. Tests cannot prove an arbitrary program correct; instructors also review compliance with the handwritten resource-management requirement. The [instructor guide](docs/instructor.md) explains reference verification, faulty-implementation regression tests, and student packaging. The [verification record](docs/verification.md) documents the framework checks already performed.

---

## 中文

本实验参考 [Stanford CS106L Assignment 7](https://github.com/cs106l/cs106l-assignments/tree/main/assignment7)，沿用“实现、应用、简答题”的分阶段作业结构，将原作业的 `unique_ptr` 主题扩展为共享所有权。你将从零实现一个小型 C++20 `lab::shared_ptr<T>`，再用它构建共享尾部的链表。

### 学习目标

练习 RAII、模板、Rule of Five、运算符重载、引用计数、异常安全和完美转发。先修知识：类、指针、`new/delete`、模板、左值/右值引用和基本异常处理。

### 快速开始

正式支持 Linux / WSL，需要支持 C++20 的 GCC 或 Clang，以及 Python 3.9+。不需要第三方 Python 包，也不需要下载测试框架。

```bash
g++ --version
python3 --version
python3 autograder/autograder.py
```

初始代码可以编译。执行未完成函数会输出 `STUDENT TODO` 并终止。初始评分预期为 **0/100、退出码 1**。每完成一部分就运行相应测试：

```bash
python3 autograder/autograder.py --list
python3 autograder/autograder.py --part basics
python3 autograder/autograder.py --case ownership.copy_constructor
python3 autograder/autograder.py --sanitize --json build/results.json
```

评分器每次从源代码重新编译，各用例在独立进程中运行。失败或崩溃不会停止后续用例。用 `--part` 或 `--case` 筛选时，分母相应调整。完整测试包含 50 个用例，每个 2 分，总分 100；简答题另由教师人工评价。详见[自动评分说明](docs/grading.md)。

### 学生文件

| 文件 | 任务 |
| --- | --- |
| `shared_ptr.h` | 完成 S1–S7 全部 `STUDENT TODO`；可添加私有字段和辅助函数。 |
| `main.cpp` | 完成 S8 的 `create_list`。 |
| `short_answer.txt` | 用自己的话回答八道简答题，每题用任一语言回答一次即可。 |

保留公开签名和已提供的辅助代码。不要通过修改测试来取得分数。`autograder/` 是公开测试，`docs/` 是实验说明，`instructor/` 仅包含框架自检，不提供完整解答。教师可提供仓库外的私有实现用于验证。

### Part 1：实现共享所有权

编码前先读[接口契约](docs/spec.md)。本实验只实现单线程、单对象、同类型共享所有权，不覆盖标准库完整 API。

1. **S1 — 状态与观察器。** 设计对象指针和共享计数的存储，实现空构造、`get`、解引用、箭头、`use_count` 和显式布尔转换。
2. **S2 — 接管裸指针。** 创建计数为 1 的控制块；控制块分配失败时回收对象。
3. **S3 — RAII。** 析构时释放本句柄的一份所有权，只有最后一个所有者删除对象和控制块。完成 S1–S3 后再期望 `basics` 全部通过。
4. **S4 — 复制。** 共享所有权，不复制 `T`。赋值需处理旧所有权、自赋值、同组赋值和 `p = p->next`。
5. **S5 — 移动。** 转移状态，使源恢复默认空状态；自移动保留原值，同组不同句柄仍需转移。
6. **S6 — 修改器。** 实现 `reset` 和 `swap`；`reset(T*)` 分配失败时保留旧所有权。
7. **S7 — 工厂。** 组合 `new T(std::forward<Args>(args)...)` 与裸指针构造函数，支持零参数和不可复制参数。

每个占位函数都有任务编号和后置条件。实现后移除相应的 `detail::todo(...)` 调用。占位符使用终止进程的方式，因为在 `noexcept` 函数中抛异常同样会终止程序。

必须手写所有权管理，不得委托给标准库或第三方智能指针。允许使用 `std::swap`、`std::exchange`、类型特征等普通工具。使用标量 `new/delete`，不支持数组。

始终维持一组一份共享计数、一个拥有者贡献一次计数的不变量。存储指针为空不等于没有所有权：字面量 `nullptr` 构造计数为 0，而传给裸指针构造函数的 `static_cast<T*>(nullptr)` 构造计数为 1。`const shared_ptr<T>` 不会让 `T` 变为 const。边界行为和异常保证以契约为准。

### Part 2：共享链表尾部

实现 `main.cpp` 中的 `create_list(values, tail)`。按照 `values` 原顺序创建新前缀，末尾共享原来的 `tail`，不得复制尾部节点。空 values 返回 tail。反向迭代器可避免无符号下标倒序时下溢。

```text
first  --> [1] --> [2] --+
                        +--> [7] --> [8]
second --> [3] ----------+
tail   -----------------+
```

修改共享节点对所有使用者可见。释放 first 后节点 1、2 被回收，其他句柄继续维持 7、8。`tail.use_count()` 统计直接持有节点 7 控制块的句柄，不统计可达路径数。

```bash
g++ -std=c++20 -Wall -Wextra -Wpedantic main.cpp -o main
./main
```

全部实现后预期输出：

```text
1 -> 2 -> 7 -> 8
3 -> 7 -> 8
tail owners: 3
```

不要构造环。链表通过成员析构递归回收，极长链表可能耗尽调用栈，测试只使用适中长度。

### 提交前检查

```bash
python3 autograder/autograder.py
python3 autograder/autograder.py --sanitize
```

两轮都应通过，并完成简答题。自动测试不能证明任意程序绝对正确；教师还会检查是否满足手写资源管理要求。[教师说明](docs/instructor.md)介绍了参考解验证、错误实现回归和学生包生成，[验证记录](docs/verification.md)列出了框架已完成的实测检查。
