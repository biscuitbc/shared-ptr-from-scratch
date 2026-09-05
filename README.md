# Shared Pointer Lab

一个模仿 [Stanford CS106L Assignment 7](https://github.com/cs106l/cs106l-assignments/tree/main/assignment7)
教学结构的独立 C++20 实验：实现共享所有权智能指针，再用它构建共享尾部的链表。
本项目并非 Stanford 官方作业；文档、代码与测试独立编写。

## 学习目标

通过模板、RAII、Rule of Five、运算符重载和完美转发，理解共享所有权。
实现后应能解释对象何时销毁，以及复制、移动和异常如何影响资源的生命周期。
先修：类、指针、`new/delete`、模板、左值/右值引用和异常的基本概念。

## 快速开始

正式支持 Linux / WSL，需支持 C++20 的 GCC 或 Clang、Python 3.9+，无第三方 Python 包或下载型测试依赖。
推荐先运行以下命令确认环境：

```bash
g++ --version
python3 --version
python3 autograder/autograder.py
```

初始代码可以编译，执行未实现的函数会输出 `STUDENT TODO` 并终止。
初始自动评分预期 **0/100、退出码 1**；这是待完成作业，不是评分器损坏。
每实现一组任务就运行相应测试：

```bash
python3 autograder/autograder.py --list
python3 autograder/autograder.py --part basics
python3 autograder/autograder.py --case ownership.copy_constructor
python3 autograder/autograder.py --sanitize --json build/results.json
```

自动评分每次从源代码重新编译，不依赖已有可执行文件。`--part` 和 `--case` 只评分选中的用例，输出对应分母。
完整评分包含 50 个用例，每个 2 分，总分 100；简答题另由教师人工评价。
评分细节、退出码和内存诊断见 [自动评分说明](docs/grading.md)。

## 文件与任务

学生只提交以下三个文件，不修改公开接口或已提供的辅助代码：

| 文件 | 工作 |
| --- | --- |
| `shared_ptr.h` | 完成 S1–S7 的所有 `STUDENT TODO`；可添加私有状态和辅助函数 |
| `main.cpp` | 完成 S8 的 `create_list` |
| `short_answer.txt` | 用自己的话回答八道简答题 |

`autograder/` 是公开测试与评分器；`docs/` 是实验说明。
`instructor/` 是教师参考实现与框架自检，不属于学生作业；教师发布时应使用打包工具排除该目录。

## Part 1：实现 shared_ptr

先阅读 [完整接口契约](docs/spec.md)。它是边界行为和评分的依据。
本实验只实现单线程、单对象、同类型所有权操作，不是标准库的完整替代品。

1. **S1：状态与观察器。** 设计对象指针和共享计数的存储，实现默认/`nullptr` 构造、`get`、解引用、箭头、`use_count`、显式布尔转换。
2. **S2：接管裸指针。** 建立计数为 1 的控制块。思考对象已分配而控制块分配失败的情况。
3. **S3：RAII。** 实现析构，只有最后一个所有者销毁对象及控制块。完成 S1–S3 后再期望 `basics` 整组通过。
4. **S4：复制。** 复制共享所有权，不复制 `T`。赋值还需要释放原有所有权，处理自赋值和来自成员的赋值。
5. **S5：移动。** 转移状态，源恢复默认空状态。自移动保留原值；不同句柄即使同组也必须正确转移。
6. **S6：修改器。** 实现 `reset` 和 `swap`。`reset(T*)` 在分配失败时必须保留旧值。
7. **S7：工厂函数。** 使用 `new T(std::forward<Args>(args)...)` 与裸指针构造函数组合，支持零参数和不可复制的参数。

每个待实现函数都有编号和后置条件。移除已完成函数中的 `detail::todo(...)` 调用。
不要在 `noexcept` 函数中抛出“未实现”异常；初始占位符使用终止进程的方式报告任务编号。
不要将管理工作委托给 `std::shared_ptr`、`std::unique_ptr`、`std::weak_ptr` 或第三方智能指针。
允许使用 `std::swap`、`std::exchange`、类型特征等普通工具。

实现时持续检查两条不变量：一个非空所有权组只有一份计数；每一个拥有该控制块的句柄贡献一次计数。
注意“有所有权”和“存储指针非空”不总相同，空的 `T*` 也可以拥有控制块。

## Part 2：共享链表尾部

完成 `main.cpp` 中的 S8：`create_list(values, tail)` 为 `values` 创建新前缀，将它连接到已有 `tail`，不复制尾部节点。
空 `values` 返回 `tail`。可以从反向迭代器开始构建，避免无符号索引倒序遍历时下溢。

```text
first  ──> [1] ──> [2] ──┐
                          ├──> [7] ──> [8]
second ──> [3] ────────────┤
tail   ───────────────────┘
```

这是共享关系；修改一个共享节点的 `value` 会对所有使用者可见。
释放 `first` 后，节点 1、2 被回收，7、8 仍由其他句柄维持。
`tail.use_count()` 只统计直接持有节点 7 控制块的句柄，不统计路径数量。

```bash
g++ -std=c++20 -Wall -Wextra -Wpedantic main.cpp -o main
./main
```

全部实现后输出：

```text
1 -> 2 -> 7 -> 8
3 -> 7 -> 8
tail owners: 3
```

不要构造环。链表按节点的成员析构递归回收，极长链表可能耗尽调用栈；本实验只测试适中长度。

## 提交前检查

```bash
python3 autograder/autograder.py
python3 autograder/autograder.py --sanitize
```

两轮都应通过，再完成简答题。自动测试不能证明任意程序绝对正确，教师还会检查实现是否符合手写资源管理要求。
教师可阅读 [维护与自检说明](docs/instructor.md)，运行参考解、自检和错误实现回归，并生成不含答案的学生包。
