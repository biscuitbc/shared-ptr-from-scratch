#include "shared_ptr.h"
#include "test.hpp"

#include <new>

using lab::shared_ptr;
using lab_test::Tracked;
using lab_memory::FailAfter;

TEST(raw_alloc_failure) {
    auto* raw = new Tracked;
    bool caught = false;
    {
        FailAfter fail(0);
        try { shared_ptr<Tracked> p(raw); }
        catch (const std::bad_alloc&) { caught = true; }
    }
    CHECK(caught && Tracked::alive == 0 && Tracked::destroyed == 1);
}

TEST(typed_null_alloc_failure) {
    bool caught = false;
    {
        FailAfter fail(0);
        try { shared_ptr<int> p(static_cast<int*>(nullptr)); }
        catch (const std::bad_alloc&) { caught = true; }
    }
    CHECK(caught);
}

TEST(reset_alloc_failure) {
    shared_ptr<Tracked> p(new Tracked(7));
    auto peer = p;
    auto* old = p.get();
    auto* fresh = new Tracked(9);
    bool caught = false;
    {
        FailAfter fail(0);
        try { p.reset(fresh); }
        catch (const std::bad_alloc&) { caught = true; }
    }
    CHECK(caught && p.get() == old && peer.get() == old);
    CHECK(p->value == 7 && p.use_count() == 2 && peer.use_count() == 2);
    CHECK(Tracked::alive == 1 && Tracked::destroyed == 1);
}

TEST(reset_empty_alloc_failure) {
    shared_ptr<Tracked> empty;
    auto* fresh = new Tracked;
    bool caught = false;
    {
        FailAfter fail(0);
        try { empty.reset(fresh); }
        catch (const std::bad_alloc&) { caught = true; }
    }
    CHECK(caught && !empty && empty.use_count() == 0);
    CHECK(Tracked::alive == 0 && Tracked::destroyed == 1);
    shared_ptr<int> typed_null(static_cast<int*>(nullptr));
    auto peer = typed_null;
    caught = false;
    {
        FailAfter fail(0);
        try { typed_null.reset(static_cast<int*>(nullptr)); }
        catch (const std::bad_alloc&) { caught = true; }
    }
    CHECK(caught && typed_null.use_count() == 2 && peer.use_count() == 2);
}

TEST(factory_object_alloc_failure) {
    bool caught = false;
    {
        FailAfter fail(0);
        try { auto p = lab::make_shared<Tracked>(1); }
        catch (const std::bad_alloc&) { caught = true; }
    }
    CHECK(caught && Tracked::alive == 0 && Tracked::destroyed == 0);
}

TEST(factory_control_alloc_failure) {
    bool caught = false;
    {
        FailAfter fail(1);
        try { auto p = lab::make_shared<Tracked>(1); }
        catch (const std::bad_alloc&) { caught = true; }
    }
    CHECK(caught && Tracked::alive == 0 && Tracked::destroyed == 1);
}

TEST(no_alloc_operations) {
    shared_ptr<Tracked> a(new Tracked(1)), b(new Tracked(2));
    shared_ptr<int> null(static_cast<int*>(nullptr));
    {
        FailAfter fail(0);
        shared_ptr<Tracked> empty, literal = nullptr;
        auto copy = a;
        auto moved = std::move(copy);
        empty = b;
        literal = std::move(empty);
        using std::swap;
        swap(a, b);
        CHECK(a->value == 2 && (*b).value == 1);
        CHECK(a.get() != b.get() && static_cast<bool>(a));
        CHECK(a.use_count() == 2 && b.use_count() == 2);
        moved.reset(); literal.reset();
        CHECK(a.use_count() == 1 && b.use_count() == 1);
        auto null_copy = null;
        CHECK(!null_copy && null_copy.use_count() == 2);
    }
    CHECK(Tracked::destroyed == 0 && null.use_count() == 1);
}
