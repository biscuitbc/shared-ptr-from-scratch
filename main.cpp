#include "shared_ptr.h"

#include <iostream>
#include <utility>
#include <vector>

// 已提供：不要修改节点定义。
template <typename T>
struct ListNode {
    T value;
    cs106l::shared_ptr<ListNode<T>> next;

    explicit ListNode(T value) : value(std::move(value)), next(nullptr) {}
};

// 返回 values 的新节点前缀，末尾直接共享 tail，保持 values 原顺序。
// values 为空时返回 tail；两者都空时返回空指针。
// T 需可拷贝构造；tail 必须是有限、无环链表。
template <typename T>
cs106l::shared_ptr<ListNode<T>> create_list(
    const std::vector<T>& values,
    cs106l::shared_ptr<ListNode<T>> tail = nullptr) {
    // STUDENT TODO [S8]: 从 values.rbegin() 到 values.rend() 构建前缀。
    // 每个新节点用 cs106l::make_shared 创建；禁止复制 tail 的节点。
    // 用 shared_ptr 的复制/移动连接节点；不要手动 delete，不要构造环。
    (void)values;
    (void)tail;
    cs106l::detail::todo("S8 create_list");
}

// 已提供：借用指针遍历，调用期间 head 维持所有权。
template <typename T>
void print_list(const cs106l::shared_ptr<ListNode<T>>& head) {
    for (auto* node = head.get(); node != nullptr; node = node->next.get()) {
        std::cout << node->value << (node->next ? " -> " : "\n");
    }
}

// 测试通过此宏复用模板；正常编译 main.cpp 时仍有独立的 main。
#ifndef SHARED_PTR_LAB_NO_MAIN
int main() {
    auto tail = create_list(std::vector<int>{7, 8});
    auto first = create_list(std::vector<int>{1, 2}, tail);
    auto second = create_list(std::vector<int>{3}, tail);
    print_list(first);
    print_list(second);
    std::cout << "tail owners: " << tail.use_count() << '\n';
}
#endif
