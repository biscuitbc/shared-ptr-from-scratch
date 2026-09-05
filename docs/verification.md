# Verification record / 验证记录

## English

Local validation on 2026-09-05 used Linux, GCC 11.4.0, Python 3.13.9, and the final suite of 50 scored cases. Complete implementations and faulty variants were kept outside the repository. This record contains results, not solutions.

| Check | Observed result |
| --- | --- |
| Pristine starter, normal mode | 50 cases compiled; all reached TODO; 0/100; exit 1 |
| Pristine starter, ASan/UBSan/leak checks | Same expected 0/100; exit 1 |
| External complete implementation, normal mode | 50/50 cases, 100/100; exit 0 |
| External complete implementation, ASan/UBSan/leak checks | 50/50 cases, 100/100; exit 0 |
| Another implementation with a different control-block layout, sanitizers | 50/50 cases, 100/100; exit 0 |
| Python framework regressions | 21/21 passed |
| Standalone completed demo | Exact documented output, including tail count 3 |
| Fault injection into complete implementations | 16/16 variants rejected by runtime checks, not compilation failures |
| Student packaging | Allowlist, pristine-source fingerprints, no-answer contents, and no-overwrite behavior verified |

The runtime fault variants covered missing copy increments, an uncleared moved-from handle, object and control-block leaks, omitted typed-null ownership, bool based on ownership instead of the stored address, pointer-only swaps, skipped same-group moves, destructive self-moves, loss of reset's old state on failure, missing constructor-failure cleanup, incorrect forwarding, late reads from a destroyed member source, reversed prefixes, discarded tails, and a decrement after the zero-count check.

The final application checks also cover copy-only value types; allocation-free operation checks exercise final-owner reset and destruction. A differential case compares 10,000 deterministic operations with standard-library behavior in the supported subset.

Reproduce public checks with `python3 instructor/verify.py`. For positive verification, supply your own external private implementation using `--solution /absolute/private-solution`. GitHub Actions checks the public framework with GCC and Clang; check the [workflow runs](https://github.com/biscuitbc/shared-ptr-from-scratch/actions/workflows/verify.yml) for the status of a specific commit.

These checks provide evidence for the documented contract. They do not constitute a proof for every possible student program or platform. Private reference implementations and mutation patches are deliberately excluded from this repository.

---

## 中文

2026-09-05 的本地验证环境为 Linux、GCC 11.4.0、Python 3.13.9，使用最终 50 项评分用例。完整实现和错误变体均保存在仓库外；本文仅记录结果，不包含解答。

| 检查 | 实测结果 |
| --- | --- |
| 原始骨架，普通模式 | 50 项均编译成功且执行到 TODO；0/100；退出码 1 |
| 原始骨架，ASan/UBSan/泄漏检测 | 同样为预期的 0/100；退出码 1 |
| 仓库外完整实现，普通模式 | 50/50 通过，100/100；退出码 0 |
| 仓库外完整实现，ASan/UBSan/泄漏检测 | 50/50 通过，100/100；退出码 0 |
| 另一种控制块布局的实现，sanitizer 模式 | 50/50 通过，100/100；退出码 0 |
| Python 框架回归 | 21/21 通过 |
| 完整实现的独立演示程序 | 精确输出文档中的结果，包括尾部计数 3 |
| 完整实现中的错误注入 | 16/16 被运行检查拒绝，均不是靠编译失败拒绝 |
| 学生打包 | 已验证白名单、原始文件指纹、不含答案及不覆盖已有文件 |

错误变体覆盖：复制漏加计数、移动后源未清空、对象及控制块泄漏、遗漏有类型空指针的所有权、用所有权而非地址判断 bool、只交换对象指针、跳过同组移动、自移动破坏状态、reset 失败丢失旧状态、构造失败漏清理、转发错误、读取已销毁的源成员、前缀顺序反转、丢弃尾部、判断零计数之后才递减。

最终应用测试还覆盖只能复制的值类型；不分配检查覆盖最后一个所有者的 reset 和析构。差分用例在支持的子集上，将 10,000 次确定性操作与标准库对照。

运行 `python3 instructor/verify.py` 可复现公开检查。正向验证请通过 `--solution /absolute/private-solution` 提供自己的仓库外私有实现。GitHub Actions 使用 GCC 和 Clang 检查公开框架，具体提交状态见[工作流运行记录](https://github.com/biscuitbc/shared-ptr-from-scratch/actions/workflows/verify.yml)。

这些检查为文档契约提供验证依据，但不是对任意学生程序或平台的正确性证明。仓库不提供私有参考实现或变体补丁。
