#include "shared_ptr.h"
#include "test.hpp"

#include <cstdint>
#include <memory>
#include <string>

TEST(value_init) {
    auto p = lab::make_shared<int>();
    CHECK(*p == 0 && p.use_count() == 1);
    auto text = lab::make_shared<std::string>();
    CHECK(text->empty() && text.use_count() == 1);
}

struct alignas(128) Wide {
    std::string text;
    int number;
    Wide(std::string s, int n) : text(std::move(s)), number(n) {}
};

TEST(multi_args) {
    auto p = lab::make_shared<Wide>(std::string(150, 'x'), 42);
    CHECK(p->text == std::string(150, 'x') && p->number == 42);
    CHECK(reinterpret_cast<std::uintptr_t>(p.get()) % alignof(Wide) == 0);
    CHECK(p.use_count() == 1);
    auto c = lab::make_shared<const int>(23);
    CHECK(*c == 23 && c.use_count() == 1);
}

struct Tag {};
struct Category {
    int kind;
    explicit Category(Tag&) : kind(1) {}
    explicit Category(const Tag&) : kind(2) {}
    explicit Category(Tag&&) : kind(3) {}
};

TEST(forwarding) {
    Tag lvalue;
    const Tag const_lvalue;
    CHECK(lab::make_shared<Category>(lvalue)->kind == 1);
    CHECK(lab::make_shared<Category>(const_lvalue)->kind == 2);
    CHECK(lab::make_shared<Category>(Tag{})->kind == 3);
    CHECK(lab::make_shared<Category>(std::move(lvalue))->kind == 3);
}

struct Immovable {
    std::unique_ptr<int> value;
    explicit Immovable(std::unique_ptr<int> p) : value(std::move(p)) {}
    Immovable(const Immovable&) = delete;
    Immovable(Immovable&&) = delete;
};

TEST(move_only) {
    auto argument = std::make_unique<int>(51);
    auto p = lab::make_shared<Immovable>(std::move(argument));
    CHECK(!argument && *p->value == 51);
    auto q = p;
    CHECK(q.get() == p.get() && q.use_count() == 2);
}

struct ConstructionError {};
struct Throwing {
    lab_test::Tracked member;
    Throwing() { throw ConstructionError{}; }
};

TEST(throwing_constructor) {
    bool caught = false;
    try { auto p = lab::make_shared<Throwing>(); }
    catch (const ConstructionError&) { caught = true; }
    CHECK(caught);
    CHECK(lab_test::Tracked::alive == 0 && lab_test::Tracked::destroyed == 1);
}
