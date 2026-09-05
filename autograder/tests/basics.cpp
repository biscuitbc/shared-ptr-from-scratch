#include "shared_ptr.h"
#include "test.hpp"

using cs106l::shared_ptr;
using lab_test::Tracked;

TEST(empty_default) {
    shared_ptr<int> p;
    CHECK(p.get() == nullptr);
    CHECK(p.use_count() == 0);
    CHECK(!p);
}

TEST(literal_null) {
    shared_ptr<int> p = nullptr;
    shared_ptr<Tracked> q{nullptr};
    CHECK(!p && !q);
    CHECK(p.get() == nullptr && q.get() == nullptr);
    CHECK(p.use_count() == 0 && q.use_count() == 0);
}

TEST(typed_null) {
    shared_ptr<int> p(static_cast<int*>(nullptr));
    CHECK(p.get() == nullptr);
    CHECK(!p);
    CHECK(p.use_count() == 1);
}

TEST(raw_observers) {
    auto* raw = new Tracked(17);
    shared_ptr<Tracked> p(raw);
    CHECK(p.get() == raw);
    CHECK(p.use_count() == 1);
    CHECK(static_cast<bool>(p));
    CHECK(&*p == raw && p.operator->() == raw);
    p->value = 29;
    CHECK((*p).value == 29);
    CHECK(p.use_count() == 1);
}

TEST(const_handle) {
    const shared_ptr<Tracked> p(new Tracked(3));
    p->value = 8;
    (*p).value += 1;
    CHECK(p.get()->value == 9);
    CHECK(p.use_count() == 1 && static_cast<bool>(p));
}

TEST(const_pointee) {
    shared_ptr<const int> p(new const int(31));
    const shared_ptr<const int>& view = p;
    CHECK(*p == 31 && *view == 31);
    CHECK(view.get() == p.get() && p.use_count() == 1);
}

TEST(destruction) {
    CHECK(Tracked::alive == 0);
    for (int i = 0; i < 1000; ++i) {
        shared_ptr<Tracked> p(new Tracked(i));
        CHECK(Tracked::alive == 1);
        CHECK(p->value == i);
    }
    CHECK(Tracked::alive == 0 && Tracked::destroyed == 1000);
    struct Marker {};
    try {
        shared_ptr<Tracked> p(new Tracked);
        throw Marker{};
    } catch (const Marker&) {}
    CHECK(Tracked::alive == 0 && Tracked::destroyed == 1001);
}
