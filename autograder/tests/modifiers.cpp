#include "shared_ptr.h"
#include "test.hpp"

using lab::shared_ptr;
using lab_test::Tracked;

TEST(reset_empty) {
    shared_ptr<int> p;
    p.reset(); p.reset();
    CHECK(!p && p.get() == nullptr && p.use_count() == 0);
}

TEST(reset_last) {
    shared_ptr<Tracked> p(new Tracked(1));
    p.reset();
    CHECK(!p && p.get() == nullptr && p.use_count() == 0);
    CHECK(Tracked::alive == 0 && Tracked::destroyed == 1);
    p.reset();
    CHECK(Tracked::destroyed == 1);
}

TEST(reset_shared) {
    shared_ptr<Tracked> p(new Tracked(2));
    auto q = p;
    p.reset();
    CHECK(!p && p.use_count() == 0);
    CHECK(q->value == 2 && q.use_count() == 1 && Tracked::destroyed == 0);
    q.reset();
    CHECK(Tracked::alive == 0 && Tracked::destroyed == 1);
}

TEST(reset_replace) {
    shared_ptr<Tracked> p(new Tracked(1));
    p.reset(new Tracked(2));
    CHECK(p->value == 2 && p.use_count() == 1 && Tracked::destroyed == 1);
    auto q = p;
    p.reset(new Tracked(3));
    CHECK(q->value == 2 && q.use_count() == 1);
    CHECK(p->value == 3 && p.use_count() == 1 && Tracked::alive == 2);
    shared_ptr<Tracked> empty;
    empty.reset(new Tracked(4));
    CHECK(empty->value == 4 && empty.use_count() == 1);
}

TEST(reset_typed_null) {
    shared_ptr<Tracked> p(new Tracked);
    p.reset(static_cast<Tracked*>(nullptr));
    CHECK(!p && p.use_count() == 1 && Tracked::destroyed == 1);
    auto q = p;
    p.reset(static_cast<Tracked*>(nullptr));
    CHECK(p.use_count() == 1 && q.use_count() == 1);
    p.reset(); q.reset();
    CHECK(p.use_count() == 0 && q.use_count() == 0);
}

TEST(swap_groups) {
    shared_ptr<Tracked> a(new Tracked(1)), b(new Tracked(2));
    auto peer = a;
    auto* old_a = a.get(); auto* old_b = b.get();
    a.swap(b);
    CHECK(a.get() == old_b && b.get() == old_a);
    CHECK(a.use_count() == 1 && b.use_count() == 2 && peer.use_count() == 2);
    CHECK(Tracked::destroyed == 0);
    b.swap(peer);
    CHECK(b.get() == old_a && b.use_count() == 2 && peer.use_count() == 2);
}

TEST(swap_empty_self_adl) {
    shared_ptr<int> a(new int(3)), b;
    using std::swap;
    swap(a, b);
    CHECK(!a && a.use_count() == 0 && *b == 3 && b.use_count() == 1);
    swap(b, b);
    CHECK(*b == 3 && b.use_count() == 1);
    swap(a, a);
    CHECK(!a && a.use_count() == 0);
    shared_ptr<int> null(static_cast<int*>(nullptr));
    swap(a, null);
    CHECK(!a && a.use_count() == 1 && !null && null.use_count() == 0);
}
