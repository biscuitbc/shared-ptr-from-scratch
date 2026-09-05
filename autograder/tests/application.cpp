#define SHARED_PTR_LAB_NO_MAIN
#include "main.cpp"
#include "test.hpp"

#include <string>

template <typename T>
std::vector<T> values_of(const lab::shared_ptr<ListNode<T>>& head) {
    std::vector<T> result;
    auto* node = head.get();
    for (int limit = 0; node && limit < 1024; ++limit) {
        result.push_back(node->value);
        node = node->next.get();
    }
    CHECK(node == nullptr);
    return result;
}

TEST(empty) {
    auto p = create_list(std::vector<int>{});
    CHECK(!p && p.get() == nullptr && p.use_count() == 0);
}

TEST(order) {
    const std::vector<int> values{1, 2, 3, 2, -1};
    auto p = create_list(values);
    CHECK(values_of(p) == values && p.use_count() == 1);
    for (auto* n = p.get(); n; n = n->next.get()) {
        if (n->next) CHECK(n->next.use_count() == 1);
    }
    auto one = create_list(std::vector<int>{6});
    CHECK(one->value == 6 && !one->next);
}

TEST(strings) {
    const std::vector<std::string> words{"", std::string(300, 'x'), "shared ownership"};
    auto p = create_list(words);
    CHECK(values_of(p) == words);
    CHECK(words[0].empty() && words[1].size() == 300);
    struct CopyOnly {
        int value;
        explicit CopyOnly(int n) : value(n) {}
        CopyOnly(const CopyOnly&) = default;
        CopyOnly(CopyOnly&&) = delete;
    };
    std::vector<CopyOnly> values;
    values.emplace_back(4);
    values.emplace_back(5);
    auto copy_only = create_list(values);
    CHECK(copy_only->value.value == 4 && copy_only->next->value.value == 5);
    CHECK(!copy_only->next->next && values[0].value == 4);
}

TEST(identity) {
    auto tail = create_list(std::vector<int>{7, 8});
    auto first = create_list(std::vector<int>{1, 2}, tail);
    auto second = create_list(std::vector<int>{3}, tail);
    CHECK(first->next->next.get() == tail.get());
    CHECK(second->next.get() == tail.get());
    CHECK(tail.use_count() == 3 && tail->next.use_count() == 1);
    CHECK(values_of(first) == std::vector<int>({1, 2, 7, 8}));
    CHECK(values_of(second) == std::vector<int>({3, 7, 8}));
    CHECK(values_of(tail) == std::vector<int>({7, 8}));
    first->next->next->value = 70;
    CHECK(second->next->value == 70 && tail->value == 70);
}

TEST(empty_with_tail) {
    auto tail = create_list(std::vector<int>{4});
    auto copy = create_list(std::vector<int>{}, tail);
    CHECK(copy.get() == tail.get() && tail.use_count() == 2);
    auto* raw = tail.get();
    auto moved = create_list(std::vector<int>{}, std::move(tail));
    CHECK(moved.get() == raw && moved.use_count() == 2);
    CHECK(!tail && tail.use_count() == 0);
}

struct ValueError {};
struct Value {
    static inline int alive = 0;
    static inline int copies_left = -1;
    int data;
    explicit Value(int v) : data(v) { ++alive; }
    Value(const Value& v) : data(v.data) {
        if (copies_left == 0) throw ValueError{};
        if (copies_left > 0) --copies_left;
        ++alive;
    }
    Value(Value&& v) noexcept : data(v.data) { ++alive; }
    ~Value() { --alive; }
};

TEST(shared_lifetime) {
    {
        const std::vector<Value> tail_values{Value(7), Value(8)};
        const std::vector<Value> prefix{Value(1), Value(2)};
        auto tail = create_list(tail_values);
        auto first = create_list(prefix, tail);
        auto second = create_list(prefix, tail);
        CHECK(Value::alive == 10);
        first.reset();
        CHECK(Value::alive == 8 && tail.use_count() == 2);
        tail.reset();
        CHECK(Value::alive == 8 && second->next->next.use_count() == 1);
        second.reset();
        CHECK(Value::alive == 4);
    }
    CHECK(Value::alive == 0);
}

TEST(prefix_exception) {
    const std::vector<Value> tail_values{Value(7)};
    const std::vector<Value> prefix{Value(1), Value(2), Value(3)};
    auto tail = create_list(tail_values);
    auto* raw = tail.get();
    const auto before = lab_memory::outstanding();
    Value::copies_left = 1;
    bool caught = false;
    try { auto p = create_list(prefix, tail); }
    catch (const ValueError&) { caught = true; }
    Value::copies_left = -1;
    CHECK(caught && tail.get() == raw && tail.use_count() == 1);
    CHECK(Value::alive == 5 && tail->value.data == 7 && !tail->next);
    CHECK(prefix[0].data == 1 && prefix[1].data == 2 && prefix[2].data == 3);
    CHECK(lab_memory::outstanding() == before);
    for (long allocation = 0; allocation < 6; ++allocation) {
        caught = false;
        {
            lab_memory::FailAfter fail(allocation);
            try { auto p = create_list(prefix, tail); }
            catch (const std::bad_alloc&) { caught = true; }
        }
        CHECK(caught && Value::alive == 5 && tail.use_count() == 1);
        CHECK(lab_memory::outstanding() == before);
    }
}

TEST(repeated_sharing) {
    auto tail = create_list(std::vector<int>{9, 10});
    for (int i = 0; i < 300; ++i) {
        auto a = create_list(std::vector<int>{i}, tail);
        auto b = create_list(std::vector<int>{i + 1}, tail);
        CHECK(a->next.get() == tail.get() && b->next.get() == tail.get());
        CHECK(tail.use_count() == 3 && a->value == i && b->value == i + 1);
    }
    CHECK(tail.use_count() == 1 && values_of(tail) == std::vector<int>({9, 10}));
    std::vector<int> sequence(256);
    for (int i = 0; i < 256; ++i) sequence[i] = i;
    auto list = create_list(sequence);
    CHECK(values_of(list) == sequence);
}
