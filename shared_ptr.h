#pragma once

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <type_traits>
#include <utility>

namespace cs106l {

namespace detail {
// TODO 占位符必须可用于 noexcept 函数，因此终止进程而不是抛异常。
// 完成一个函数后，移除该函数对 todo() 的调用；不要修改这个辅助函数。
[[noreturn]] inline void todo(const char* task) noexcept {
    std::fprintf(stderr, "STUDENT TODO: %s\n", task);
    std::abort();
}
} // namespace detail

// 教学子集：单线程、单对象、同类型共享所有权。完整契约见 docs/spec.md。
template <typename T>
class shared_ptr {
    static_assert(std::is_object_v<T> && !std::is_array_v<T>,
                  "This lab supports non-array object types only.");

private:
    // STUDENT TODO [S1]: 设计对象指针与共享计数的存储。
    // 空状态不分配内存；同组的所有 shared_ptr 必须访问同一份计数。
    // 可添加私有辅助函数，但不能用标准库智能指针代理资源管理。

public:
    using element_type = T;

    shared_ptr() noexcept {
        // STUDENT TODO [S1]: get() == nullptr, use_count() == 0。
        detail::todo("S1 default constructor");
    }

    shared_ptr(std::nullptr_t) noexcept {
        // STUDENT TODO [S1]: 与默认构造相同；允许 shared_ptr<T> p = nullptr。
        detail::todo("S1 nullptr constructor");
    }

    explicit shared_ptr(T* ptr) {
        // STUDENT TODO [S2]: 接管 ptr，创建计数为 1 的控制块。
        // 即使 ptr 是空的 T*，也创建控制块。
        // 控制块分配失败时 delete ptr 并重新抛出异常。
        (void)ptr;
        detail::todo("S2 raw pointer constructor");
    }

    ~shared_ptr() noexcept {
        // STUDENT TODO [S3]: 释放本句柄的一份所有权。
        // 只有最后一个所有者释放对象和控制块；空句柄不做释放。
        detail::todo("S3 destructor");
    }

    shared_ptr(const shared_ptr& other) noexcept {
        // STUDENT TODO [S4]: 共享对象及控制块，非空所有权组计数加一。
        (void)other;
        detail::todo("S4 copy constructor");
    }

    shared_ptr& operator=(const shared_ptr& other) noexcept {
        // STUDENT TODO [S4]: 接管 other 的一份共享所有权，释放旧所有权。
        // 覆盖自赋值、同组赋值和 p = p->next；返回 *this。
        (void)other;
        detail::todo("S4 copy assignment");
    }

    shared_ptr(shared_ptr&& other) noexcept {
        // STUDENT TODO [S5]: 转移所有权，不增加计数，使 other 变为默认空状态。
        (void)other;
        detail::todo("S5 move constructor");
    }

    shared_ptr& operator=(shared_ptr&& other) noexcept {
        // STUDENT TODO [S5]: 转移所有权并释放旧所有权；返回 *this。
        // 自移动必须保留原值；还要处理 p = std::move(p->next)。
        (void)other;
        detail::todo("S5 move assignment");
    }

    T* get() const noexcept {
        // STUDENT TODO [S1]: 返回存储的指针，不改变计数。
        detail::todo("S1 get");
    }

    T& operator*() const noexcept {
        // STUDENT TODO [S1]: 前置条件 get() != nullptr。
        // const shared_ptr<T> 不代表 const T；不要返回 const T&。
        detail::todo("S1 dereference");
    }

    T* operator->() const noexcept {
        // STUDENT TODO [S1]: 前置条件 get() != nullptr。
        detail::todo("S1 arrow");
    }

    long use_count() const noexcept {
        // STUDENT TODO [S1]: 无控制块返回 0，否则返回共享计数。
        detail::todo("S1 use_count");
    }

    explicit operator bool() const noexcept {
        // STUDENT TODO [S1]: 等价于 get() != nullptr，与计数是否为 0 无关。
        detail::todo("S1 bool conversion");
    }

    void reset() noexcept {
        // STUDENT TODO [S6]: 释放本句柄所有权，恢复默认空状态。
        detail::todo("S6 reset");
    }

    void reset(T* ptr) {
        // STUDENT TODO [S6]: 等价于 shared_ptr(ptr).swap(*this)。
        // 新控制块分配失败时，旧所有权必须保持不变，ptr 必须被删除。
        // ptr 不得是已被任何所有权组管理的非空指针。
        (void)ptr;
        detail::todo("S6 reset pointer");
    }

    void swap(shared_ptr& other) noexcept {
        // STUDENT TODO [S6]: 交换整个所有权状态；不分配、不销毁对象、不改计数。
        (void)other;
        detail::todo("S6 swap");
    }
};

// 已提供的 ADL swap，无需修改。
template <typename T>
void swap(shared_ptr<T>& lhs, shared_ptr<T>& rhs) noexcept {
    lhs.swap(rhs);
}

template <typename T, typename... Args>
shared_ptr<T> make_shared(Args&&... args) {
    // STUDENT TODO [S7]: new T(...) + 上面的原始指针构造函数。
    // 使用 std::forward 完美转发；无参数时值初始化。
    // 本实验要求对象和控制块分别分配，不要求标准库常见的合并分配优化。
    (void)sizeof...(args);
    detail::todo("S7 make_shared");
}

} // namespace cs106l
