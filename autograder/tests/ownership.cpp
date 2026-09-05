#include "shared_ptr.h"
#include "test.hpp"

#include <array>
#include <cstdint>
#include <memory>

using lab::shared_ptr;
using lab_test::Tracked;

TEST(copy_constructor) {
    shared_ptr<Tracked> survivor;
    {
        shared_ptr<Tracked> original(new Tracked(41));
        const auto& view = original;
        shared_ptr<Tracked> copy(view);
        CHECK(copy.get() == original.get());
        CHECK(copy.use_count() == 2 && original.use_count() == 2);
        copy->value = 73;
        CHECK(original->value == 73);
        survivor = copy;
        CHECK(survivor.use_count() == 3);
    }
    CHECK(survivor.use_count() == 1 && survivor->value == 73);
    CHECK(Tracked::alive == 1 && Tracked::destroyed == 0);
}

TEST(copy_empty) {
    shared_ptr<int> a;
    shared_ptr<int> b(a);
    CHECK(!a && !b && a.get() == b.get());
    CHECK(a.use_count() == 0 && b.use_count() == 0);
}

TEST(copy_typed_null) {
    shared_ptr<int> a(static_cast<int*>(nullptr));
    {
        auto b = a;
        CHECK(!a && !b);
        CHECK(a.use_count() == 2 && b.use_count() == 2);
    }
    CHECK(a.use_count() == 1);
}

TEST(copy_assignment_releases) {
    shared_ptr<Tracked> a(new Tracked(1)), b(new Tracked(2));
    CHECK(&(a = b) == &a);
    CHECK(a.get() == b.get() && a->value == 2);
    CHECK(a.use_count() == 2 && b.use_count() == 2);
    CHECK(Tracked::alive == 1 && Tracked::destroyed == 1);
}

TEST(copy_assignment_shared_target) {
    shared_ptr<Tracked> a(new Tracked(1));
    auto old_peer = a;
    shared_ptr<Tracked> b(new Tracked(2));
    a = b;
    CHECK(old_peer->value == 1 && old_peer.use_count() == 1);
    CHECK(a->value == 2 && a.use_count() == 2 && b.use_count() == 2);
    CHECK(Tracked::alive == 2 && Tracked::destroyed == 0);
}

TEST(copy_assignment_empty) {
    shared_ptr<Tracked> a(new Tracked), empty;
    a = empty;
    CHECK(!a && a.use_count() == 0 && a.get() == nullptr);
    CHECK(!empty && empty.use_count() == 0);
    CHECK(Tracked::alive == 0 && Tracked::destroyed == 1);
    a = empty;
    shared_ptr<Tracked> b(new Tracked(9));
    empty = b;
    CHECK(empty->value == 9 && empty.use_count() == 2);
}

TEST(copy_self_and_same_group) {
    shared_ptr<Tracked> a(new Tracked(8));
    auto* raw = a.get();
    const auto& alias = a;
    CHECK(&(a = alias) == &a);
    CHECK(a.get() == raw && a.use_count() == 1);
    auto b = a;
    a = b;
    CHECK(a.get() == raw && b.use_count() == 2 && a.use_count() == 2);
    CHECK(Tracked::destroyed == 0);
    shared_ptr<Tracked> empty;
    const auto& empty_alias = empty;
    empty = empty_alias;
    CHECK(!empty && empty.use_count() == 0);
}

TEST(move_constructor) {
    shared_ptr<Tracked> a(new Tracked(9));
    auto peer = a;
    auto* raw = a.get();
    shared_ptr<Tracked> b(std::move(a));
    CHECK(!a && a.get() == nullptr && a.use_count() == 0);
    CHECK(b.get() == raw && b.use_count() == 2 && peer.use_count() == 2);
    CHECK(b->value == 9 && Tracked::destroyed == 0);
    a = b;
    CHECK(a.get() == raw && a.use_count() == 3);
}

TEST(move_assignment) {
    shared_ptr<Tracked> a(new Tracked(1)), b(new Tracked(2));
    auto peer = b;
    auto* raw = b.get();
    CHECK(&(a = std::move(b)) == &a);
    CHECK(!b && b.get() == nullptr && b.use_count() == 0);
    CHECK(a.get() == raw && a->value == 2 && a.use_count() == 2);
    CHECK(peer.use_count() == 2 && Tracked::destroyed == 1);
    auto old_peer = a;
    shared_ptr<Tracked> c(new Tracked(3));
    a = std::move(c);
    CHECK(a->value == 3 && a.use_count() == 1);
    CHECK(old_peer->value == 2 && peer.use_count() == 2);
}

TEST(move_empty_and_null) {
    shared_ptr<int> empty;
    shared_ptr<int> moved(std::move(empty));
    CHECK(!moved && moved.use_count() == 0 && empty.use_count() == 0);
    shared_ptr<int> a(new int(5));
    a = std::move(moved);
    CHECK(!a && a.use_count() == 0 && moved.use_count() == 0);
    shared_ptr<int> null(static_cast<int*>(nullptr));
    shared_ptr<int> n(std::move(null));
    CHECK(!n && n.use_count() == 1 && null.use_count() == 0);
    a = std::move(n);
    CHECK(!a && a.use_count() == 1 && n.use_count() == 0);
    shared_ptr<int> live(new int(6));
    moved = std::move(live);
    CHECK(*moved == 6 && moved.use_count() == 1 && !live);
}

TEST(move_self_and_same_group) {
    shared_ptr<Tracked> a(new Tracked(4));
    auto* raw = a.get();
    auto& alias = a;
    CHECK(&(a = std::move(alias)) == &a);
    CHECK(a.get() == raw && a.use_count() == 1 && a->value == 4);
    auto b = a;
    a = std::move(b);
    CHECK(a.get() == raw && a.use_count() == 1);
    CHECK(!b && b.use_count() == 0 && Tracked::destroyed == 0);
    shared_ptr<Tracked> empty;
    auto& empty_alias = empty;
    empty = std::move(empty_alias);
    CHECK(!empty && empty.use_count() == 0);
    shared_ptr<int> null(static_cast<int*>(nullptr));
    auto& null_alias = null;
    null = std::move(null_alias);
    CHECK(!null && null.use_count() == 1);
}

struct Node {
    static inline int alive = 0;
    int value;
    shared_ptr<Node> next;
    explicit Node(int n) : value(n) { ++alive; }
    ~Node() { --alive; }
};

TEST(nested_assignment) {
    for (bool move : {false, true}) {
        shared_ptr<Node> p(new Node(1));
        p->next = shared_ptr<Node>(new Node(2));
        p->next->next = shared_ptr<Node>(new Node(3));
        auto* second = p->next.get();
        if (move) p = std::move(p->next);
        else p = p->next;
        CHECK(Node::alive == 2 && p.get() == second);
        CHECK(p->value == 2 && p->next->value == 3 && p.use_count() == 1);
    }
    CHECK(Node::alive == 0);
}

TEST(differential_sequences) {
    // Compare only the supported subset against independent standard-library objects.
    // 仅在支持的子集上，与独立的标准库对象对照行为。
    std::array<shared_ptr<int>, 8> actual;
    std::array<std::shared_ptr<int>, 8> expected;
    std::uint32_t state = 0x106107u;
    auto draw = [&]() { state = state * 1664525u + 1013904223u; return state; };
    for (int step = 0; step < 10000; ++step) {
        const auto i = (draw() >> 16) % actual.size();
        const auto j = (draw() >> 16) % actual.size();
        switch ((draw() >> 16) % 10) {
        case 0: actual[i] = actual[j]; expected[i] = expected[j]; break;
        case 1: actual[i] = std::move(actual[j]); expected[i] = std::move(expected[j]); break;
        case 2: actual[i].reset(); expected[i].reset(); break;
        case 3: actual[i].reset(new int(step)); expected[i].reset(new int(step)); break;
        case 4: actual[i].swap(actual[j]); expected[i].swap(expected[j]); break;
        case 5: actual[i] = nullptr; expected[i] = nullptr; break;
        case 6:
            actual[i].reset(static_cast<int*>(nullptr));
            expected[i].reset(static_cast<int*>(nullptr));
            break;
        case 7:
            if (actual[i]) *actual[i] += 1;
            if (expected[i]) *expected[i] += 1;
            break;
        case 8: {
            auto a = actual[j]; auto e = expected[j];
            actual[i] = a; expected[i] = e;
            break;
        }
        case 9: {
            auto a = std::move(actual[j]); auto e = std::move(expected[j]);
            actual[i] = std::move(a); expected[i] = std::move(e);
            break;
        }
        }
        for (std::size_t k = 0; k < actual.size(); ++k) {
            CHECK(static_cast<bool>(actual[k]) == static_cast<bool>(expected[k]));
            CHECK(actual[k].use_count() == expected[k].use_count());
            if (actual[k]) CHECK(*actual[k] == *expected[k]);
            for (std::size_t l = 0; l < actual.size(); ++l) {
                CHECK((actual[k].get() == actual[l].get()) ==
                      (expected[k].get() == expected[l].get()));
            }
        }
    }
}
