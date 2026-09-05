# Grading guide / 自动评分说明

## English

### Score and coverage

Each of the 50 cases is worth 2 points. A case earns both points only if all its checks pass; no points are awarded for merely compiling. Short answers are reviewed manually outside the 100-point automatic score.

| Suite | Cases | Points | Coverage |
| --- | ---: | ---: | --- |
| `basics` | 7 | 14 | Empty states, typed null, observers, const correctness, destruction and unwinding |
| `ownership` | 13 | 26 | Copy/move, shared/empty sources and targets, self/same-group operations, member sources, differential sequences |
| `modifiers` | 7 | 14 | Reset, replacement, surviving peers, typed-null groups, member/ADL swap |
| `factory` | 5 | 10 | Value initialization, multiple arguments, alignment, forwarding, noncopyable arguments, constructor exceptions |
| `application` | 8 | 16 | Empty/single/multiple nodes, strings, shared identity, lifetime, prefix failures, repeated sharing |
| `exceptions` | 7 | 14 | Failure of object/control-block allocation, strong reset guarantee, allocation-free operations |
| `interface` | 3 | 6 | Compile-time contracts, multiple translation units, eleven rejected programs |
| Total | 50 | 100 | All cases are public |

The canonical inventory is `autograder/cases.py`; executable case registration is checked against it. `--list` shows every case and its hint. The deterministic differential test runs 10,000 operations over eight handles, checking pointer-equivalence relationships, values, truthiness, and counts against independent `std::shared_ptr` objects after each operation.

Some cases depend on multiple tasks. For example, ownership tests need valid construction and destruction, and the differential case also uses modifiers. A suite is one compilation unit: a syntax/signature error in that suite gives zero for its selected cases while other suites continue. An error in `main.cpp` does not prevent core suites from compiling. The `interface` suite links a second translation unit.

The `interface.rejected_programs` case first requires a successful runtime check, then verifies eleven small programs fail compilation: implicit raw-pointer, bool, integer and pointer conversions; modification through a const pointee; unbounded/bounded arrays; void, reference and function specializations; and `reset(nullptr)`. This prevents an invalid header that rejects everything from earning credit.

### Commands

```bash
python3 autograder/autograder.py
python3 autograder/autograder.py --part ownership
python3 autograder/autograder.py --case 'ownership.copy_*'
python3 autograder/autograder.py --cxx clang++ --sanitize
python3 autograder/autograder.py --submission /absolute/student-directory --json build/results.json
```

`--part` and `--case` may be repeated; combining them takes the intersection. Patterns must match real case IDs, and an empty selection is an error. Filtered runs use their actual maximum score, not 100. `--list` lists cases without compiling or writing a grade report. `--cxx` / `CXX` select a single compiler executable, not a shell command with extra flags.

`--timeout` defaults to 5 seconds per case; `--compile-timeout` defaults to 60 seconds per compilation. Both must be positive finite numbers. Use `--quiet` to show only the final total and retain diagnostics in `--json`.

### Isolation and memory checks

Every run compiles fresh binaries in a new temporary directory. Only `shared_ptr.h` and `main.cpp` are copied from the submission; the tests come from the trusted framework. No stale executable is reused, and compilation commands do not invoke a shell.

Each case executes in a separate process. Assertions, exceptions, signals, timeouts, and nonzero exits fail that case. Exit code 0 alone is insufficient: the harness must also print the matching completion marker. Timeouts terminate the process group on POSIX. Captured output is spooled to disk and the retained diagnostic is bounded. This is a local teaching runner, not a security sandbox for hostile submissions.

All cases track C++ allocation balance, including scalar/array, aligned, sized-delete, and nothrow paths. A leaked object or control block causes a nonzero allocation delta. Object-lifetime checks additionally verify destruction counts. This accounting does not replace a memory sanitizer: it does not diagnose every out-of-bounds access, arbitrary `malloc` leak, or equal-sized lifetime error.

`--sanitize` adds AddressSanitizer, UndefinedBehaviorSanitizer, and leak detection. It also retains deterministic allocation-failure injection. The grader probes the compiler and sanitizer runtime first; unsupported infrastructure is reported separately from student failures. Linux sanitizer binaries disable PIE to avoid shadow-address collisions on affected hosts. The supported platform is Linux / WSL; other platforms have not been validated.

### Exit codes and JSON

| Code | Meaning |
| --- | --- |
| 0 | Every selected case passed, or a successful `--list` |
| 1 | At least one selected student case failed, timed out, or could not compile |
| 2 | Invalid CLI/configuration, missing compiler/source, inconsistent registry, unavailable runtime, or report-writing failure |

Completed grading reports contain `schema_version`, `status`, `submission`, `compiler`, `sanitized`, `score`, `max_score`, `passed`, `failed`, and `tests`. Each test contains `name`, `part`, `status`, `score`, `max_score`, `hint`, and `output`. Test status is `passed`, `failed`, `timeout`, or `compile_error`.

Report `status: completed` means grading completed, not that the submission passed. Infrastructure errors use `status: infrastructure_error` and an `error` field, with no numeric grade. Reports atomically replace the requested output file, so a configuration error does not leave an old successful report behind after argument parsing. Invalid argument syntax is handled by argparse before report generation.

The initial TODO skeleton must compile every suite, fail all 50 cases with a TODO diagnostic, score 0/100, and exit 1. Tests do not assign a correctness proof or replace manual review of the required handwritten implementation.

---

## 中文

### 分数与覆盖范围

50 个用例各 2 分，每个用例全部检查通过才得分，单纯编译成功不得分。简答题另行人工评价，不计入自动评分的 100 分。

| 测试组 | 用例数 | 分数 | 覆盖内容 |
| --- | ---: | ---: | --- |
| `basics` | 7 | 14 | 空状态、有类型空指针、观察器、const 正确性、正常和异常析构 |
| `ownership` | 13 | 26 | 复制/移动、共享/空源和目标、自身/同组操作、成员源、差分序列 |
| `modifiers` | 7 | 14 | 清空、替换、保留其他所有者、空指针所有权组、成员及 ADL 交换 |
| `factory` | 5 | 10 | 值初始化、多参数、对齐、完美转发、不可复制参数、构造异常 |
| `application` | 8 | 16 | 空/单/多节点、字符串、共享身份、生命周期、前缀失败、重复共享 |
| `exceptions` | 7 | 14 | 对象/控制块分配失败、reset 强异常保证、不分配操作 |
| `interface` | 3 | 6 | 编译期契约、多翻译单元、十一项错误代码拒绝检查 |
| 合计 | 50 | 100 | 全部用例公开 |

唯一清单位于 `autograder/cases.py`，运行时会核对可执行文件注册的用例。`--list` 可查看完整列表和提示。固定种子的差分测试对八个句柄执行 10,000 次操作，每步与独立的 `std::shared_ptr` 对照指针等价关系、对象值、布尔值和计数。

部分用例依赖多个任务，例如所有权测试需要构造与析构正确，差分测试还使用修改器。每组独立编译：组内语法或签名错误使该组被选中的用例得零分，其他组继续。`main.cpp` 中的错误不妨碍核心组编译。`interface` 组还链接第二个翻译单元。

`interface.rejected_programs` 先要求运行检查成功，再验证十一段程序编译失败：裸指针、bool、整数和指针隐式转换；通过 const 所指对象修改值；无界/定长数组；void、引用、函数类型；以及 `reset(nullptr)`。因此，一个拒绝一切代码的损坏头文件不会因此得分。

### 运行命令

```bash
python3 autograder/autograder.py
python3 autograder/autograder.py --part ownership
python3 autograder/autograder.py --case 'ownership.copy_*'
python3 autograder/autograder.py --cxx clang++ --sanitize
python3 autograder/autograder.py --submission /absolute/student-directory --json build/results.json
```

`--part` 和 `--case` 可重复，组合时取交集。通配符必须匹配真实用例，空选择会报错。筛选评分使用实际分母。`--list` 不编译也不生成成绩。`--cxx` / `CXX` 指定一个编译器可执行文件，不能填包含额外参数的 shell 命令。

单用例 `--timeout` 默认为 5 秒，单次编译 `--compile-timeout` 默认为 60 秒，必须是有限正数。`--quiet` 只输出总分，详细诊断可通过 `--json` 保存。

### 隔离与内存检查

每次在新的临时目录从源代码编译，仅从提交目录复制 `shared_ptr.h` 和 `main.cpp`，测试始终来自可信框架。不会复用旧可执行文件，编译命令不经过 shell。

每个用例在独立进程中执行。断言、异常、信号、超时和非零退出都会失败。仅退出码为 0 不足以通过，还必须由框架输出匹配的完成标记。POSIX 超时会终止进程组。输出暂存磁盘，保留的诊断长度有限。本工具用于本地教学运行，不是执行恶意提交的安全沙箱。

全部用例统计 C++ 分配余额，覆盖标量/数组、对齐、带大小 delete 和 nothrow 路径。泄漏对象或控制块会造成分配余额非零；对象生命周期测试还核对析构次数。余额检查不能替代内存检测器，不能诊断全部越界、任意 `malloc` 泄漏或分配数量相抵的生命周期错误。

`--sanitize` 启用 AddressSanitizer、UndefinedBehaviorSanitizer 和泄漏检测，并保留确定性的分配失败注入。评分前先探测编译器和检测器运行环境，环境不支持会单独报告。Linux 检测器构建关闭 PIE，避免部分主机的影子地址冲突。正式支持 Linux / WSL，其他平台未验证。

### 退出码与 JSON

| 退出码 | 含义 |
| --- | --- |
| 0 | 全部所选用例通过，或 `--list` 成功 |
| 1 | 至少一个学生用例失败、超时或无法编译 |
| 2 | 参数/配置错误、缺少编译器或源文件、用例注册不一致、运行环境不可用或报告写入失败 |

完成评分的报告包含 `schema_version`、`status`、`submission`、`compiler`、`sanitized`、`score`、`max_score`、`passed`、`failed` 和 `tests`。每项包含 `name`、`part`、`status`、`score`、`max_score`、`hint`、`output`；用例状态为 `passed`、`failed`、`timeout` 或 `compile_error`。

报告中的 `status: completed` 代表评分已执行完毕，不代表全部通过。基础设施错误使用 `status: infrastructure_error` 和 `error` 字段，不生成数字成绩。参数解析后，指定报告会原子替换，因此配置错误不会留下此前的成功成绩。参数语法错误由 argparse 在生成报告前处理。

初始 TODO 骨架必须各组都能编译，50 项均报告 TODO 失败，得分 0/100，退出码 1。测试不能证明任意程序绝对正确，也不能替代手写实现要求的人工检查。
