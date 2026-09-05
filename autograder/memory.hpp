#pragma once

namespace lab_memory {
long outstanding() noexcept;
long exchange_failure(long after) noexcept;

// Fail once after N successful C++ allocations, then restore normal allocation.
// N 次 C++ 分配成功后失败一次，然后恢复正常分配。
// The guard also restores the previous setting on every exception path.
// 守卫也会在所有异常路径恢复此前设置。
class FailAfter {
    long previous_;
public:
    explicit FailAfter(long after) noexcept : previous_(exchange_failure(after)) {}
    ~FailAfter() { exchange_failure(previous_); }
    FailAfter(const FailAfter&) = delete;
    FailAfter& operator=(const FailAfter&) = delete;
};
} // namespace lab_memory / 命名空间 lab_memory
