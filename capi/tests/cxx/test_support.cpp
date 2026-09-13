#include "scid/libscid/support.h"

#include <cassert>
#include <cstddef>
#include <string_view>

using namespace scid::libscid;

template <typename... Args>
concept NullCheckable = requires(Args... args) {
    any_null(args...);
};

static_assert(NullCheckable<int*>);
static_assert(NullCheckable<const char*, void*, int**>);
static_assert(NullCheckable<std::nullptr_t>);
static_assert(NullCheckable<int*, std::nullptr_t>);

static_assert(!NullCheckable<int>);
static_assert(!NullCheckable<int*, int>);
static_assert(!NullCheckable<std::string_view>);

namespace
{
    constexpr int a = 1;
    constexpr int b = 2;
    constexpr int c = 3;

    static_assert(!any_null());
    static_assert(!any_null(&a));
    static_assert(any_null(static_cast<const int*>(nullptr)));
    static_assert(any_null(nullptr));

    static_assert(!any_null(&a, &b));
    static_assert(!any_null(&a, &b, &c));

    static_assert(any_null(nullptr, &b, &c));
    static_assert(any_null(&a, nullptr, &c));
    static_assert(any_null(&a, &b, nullptr));
    static_assert(any_null(nullptr, nullptr, nullptr));
}

int
main()
{
    int val_a = 10;
    int val_b = 20;

    int* p_a = &val_a;
    int* p_b = &val_b;
    int* p_null = nullptr;

    assert(!any_null());
    assert(!any_null(p_a));
    assert(any_null(p_null));

    assert(!any_null(p_a, p_b));
    assert(any_null(p_null, p_b));
    assert(any_null(p_a, p_null));
    assert(any_null(p_null, p_null));

    const char* str = "test";
    void* raw = &val_a;
    assert(!any_null(str, raw, p_a));
    assert(any_null(str, nullptr, p_a));

    return 0;
}
