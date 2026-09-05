#include "shared_ptr.h"
#include "test.hpp"

#include <type_traits>
#include <utility>

using P = lab::shared_ptr<int>;
using CP = lab::shared_ptr<const int>;

static_assert(std::is_same_v<P::element_type, int>);
static_assert(std::is_nothrow_default_constructible_v<P>);
static_assert(std::is_nothrow_constructible_v<P, std::nullptr_t>);
static_assert(std::is_nothrow_destructible_v<P>);
static_assert(std::is_nothrow_copy_constructible_v<P>);
static_assert(std::is_nothrow_copy_assignable_v<P>);
static_assert(std::is_nothrow_move_constructible_v<P>);
static_assert(std::is_nothrow_move_assignable_v<P>);
static_assert(!std::is_nothrow_constructible_v<P, int*>);
static_assert(!std::is_convertible_v<int*, P>);
static_assert(std::is_convertible_v<std::nullptr_t, P>);
static_assert(!std::is_convertible_v<P, bool>);
static_assert(!std::is_convertible_v<P, int>);
static_assert(!std::is_convertible_v<P, int*>);
static_assert(std::is_same_v<decltype(std::declval<const P&>().get()), int*>);
static_assert(std::is_same_v<decltype(*std::declval<const P&>()), int&>);
static_assert(std::is_same_v<decltype(std::declval<const P&>().operator->()), int*>);
static_assert(std::is_same_v<decltype(std::declval<const P&>().use_count()), long>);
static_assert(std::is_same_v<decltype(*std::declval<const CP&>()), const int&>);
static_assert(std::is_same_v<decltype(std::declval<const CP&>().get()), const int*>);
static_assert(std::is_same_v<decltype(std::declval<P&>() = std::declval<const P&>()), P&>);
static_assert(std::is_same_v<decltype(std::declval<P&>() = std::declval<P&&>()), P&>);
static_assert(std::is_same_v<decltype(std::declval<P&>().reset()), void>);
static_assert(std::is_same_v<decltype(std::declval<P&>().reset(std::declval<int*>())), void>);
static_assert(std::is_same_v<decltype(std::declval<P&>().swap(std::declval<P&>())), void>);
static_assert(noexcept(std::declval<const P&>().get()));
static_assert(noexcept(*std::declval<const P&>()));
static_assert(noexcept(std::declval<const P&>().operator->()));
static_assert(noexcept(std::declval<const P&>().use_count()));
static_assert(noexcept(static_cast<bool>(std::declval<const P&>())));
static_assert(noexcept(std::declval<P&>().reset()));
static_assert(!noexcept(std::declval<P&>().reset(std::declval<int*>())));
static_assert(noexcept(std::declval<P&>().swap(std::declval<P&>())));
static_assert(noexcept(lab::swap(std::declval<P&>(), std::declval<P&>())));
static_assert(std::is_nothrow_swappable_v<P>);

P from_other_translation_unit();

TEST(api_contract) {
    const P p(new int(4));
    *p = 5;
    CHECK(*p == 5 && p.use_count() == 1);
    if (p) CHECK(p.get() != nullptr);
    else CHECK(false);
}

TEST(multiple_translation_units) {
    auto p = from_other_translation_unit();
    CHECK(*p == 106 && p.use_count() == 1);
    auto q = lab::make_shared<int>(107);
    lab::swap(p, q);
    CHECK(*p == 107 && *q == 106);
}

TEST(rejected_programs) {
    // The Python runner also compiles eleven forbidden programs for this case.
    // Python 评分器还会为本用例编译十一段应被拒绝的代码。
    P p;
    CHECK(!p && p.use_count() == 0);
}
