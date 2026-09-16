#include "scid/libscid/support.h"

#include "test_libscid.h"

#include <cassert>
#include <concepts>
#include <cstddef>
#include <new>
#include <stdexcept>
#include <string_view>
#include <utility>

using namespace scid::libscid;

template <typename... Args>
concept NullCheckable = requires(Args... args) { any_null(args...); };

template <typename F>
concept AbiGuardable = requires(F&& fn) {
    { abi_guard(std::forward<F>(fn)) } -> std::same_as<scid_error>;
};

template <typename F>
concept AbiGuardVoidable = requires(F&& fn) {
    { abi_guard_void(std::forward<F>(fn)) } -> std::same_as<void>;
};

static_assert(NullCheckable<int*>);
static_assert(NullCheckable<
              const char*,
              void*,
              int**>);
static_assert(NullCheckable<std::nullptr_t>);
static_assert(NullCheckable<
              int*,
              std::nullptr_t>);

static_assert(!NullCheckable<int>);
static_assert(!NullCheckable<
              int*,
              int>);
static_assert(!NullCheckable<std::string_view>);

static_assert(AbiGuardable<scid_error (*)()>);
static_assert(AbiGuardable<decltype([] { return SCID_OK; })>);
static_assert(AbiGuardable<decltype([] { return SCID_ERROR; })>);
static_assert(!AbiGuardable<void (*)()>);
static_assert(!AbiGuardable<decltype([](int) { return SCID_OK; })>);
static_assert(!AbiGuardable<int>);

static_assert(AbiGuardVoidable<void (*)()>);
static_assert(AbiGuardVoidable<decltype([] {})>);
static_assert(!AbiGuardVoidable<decltype([](int) {})>);
static_assert(!AbiGuardVoidable<int>);

static_assert(noexcept(abi_guard([] { return SCID_OK; })));
static_assert(noexcept(abi_guard_void([] {})));

namespace
{
    constexpr int a = 1;
    constexpr int b = 2;
    constexpr int c = 3;

    static_assert(!any_null());
    static_assert(!any_null(&a));
    static_assert(any_null(static_cast<const int*>(nullptr)));
    static_assert(any_null(nullptr));

    static_assert(!any_null(
        &a,
        &b));
    static_assert(!any_null(
        &a,
        &b,
        &c));

    static_assert(any_null(
        nullptr,
        &b,
        &c));
    static_assert(any_null(
        &a,
        nullptr,
        &c));
    static_assert(any_null(
        &a,
        &b,
        nullptr));
    static_assert(any_null(
        nullptr,
        nullptr,
        nullptr));

    void
    test_any_null_runtime()
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
        void*       raw = &val_a;
        assert(!any_null(str, raw, p_a));
        assert(any_null(str, nullptr, p_a));
    }

    void
    test_abi_guard_success()
    {
        const scid_error ok_res = abi_guard([] { return SCID_OK; });
        assert(ok_res == SCID_OK);

        const scid_error err_res = abi_guard([] { return SCID_ERROR_INVALID_FEN; });
        assert(err_res == SCID_ERROR_INVALID_FEN);
    }

    void
    test_abi_guard_bad_alloc()
    {
        const scid_error res = abi_guard([]() -> scid_error { throw std::bad_alloc(); });
        assert(res == SCID_ERROR_NO_MEMORY);
    }

    void
    test_abi_guard_out_of_range()
    {
        const scid_error res =
            abi_guard([]() -> scid_error { throw std::out_of_range("index out of bounds"); });
        assert(res == SCID_ERROR_BAD_ARG);
    }

    void
    test_abi_guard_invalid_argument()
    {
        const scid_error res =
            abi_guard([]() -> scid_error { throw std::invalid_argument("invalid parameter"); });
        assert(res == SCID_ERROR_BAD_ARG);
    }

    void
    test_abi_guard_length_error()
    {
        const scid_error res =
            abi_guard([]() -> scid_error { throw std::length_error("buffer length exceeded"); });
        assert(res == SCID_ERROR);
    }

    void
    test_abi_guard_generic_exception()
    {
        const scid_error res =
            abi_guard([]() -> scid_error { throw std::runtime_error("general failure"); });
        assert(res == SCID_ERROR);
    }

    void
    test_abi_guard_unknown_exception()
    {
        const scid_error res = abi_guard([]() -> scid_error { throw 42; });
        assert(res == SCID_ERROR);
    }

    void
    test_abi_guard_void()
    {
        bool executed = false;
        abi_guard_void([&] { executed = true; });
        assert(executed);

        abi_guard_void([] { throw std::bad_alloc(); });
        abi_guard_void([] { throw std::runtime_error("failed cleanup"); });
        abi_guard_void([] { throw "foreign error"; });
    }
}

void
test_support()
{
    test_any_null_runtime();
    test_abi_guard_success();
    test_abi_guard_bad_alloc();
    test_abi_guard_out_of_range();
    test_abi_guard_invalid_argument();
    test_abi_guard_length_error();
    test_abi_guard_generic_exception();
    test_abi_guard_unknown_exception();
    test_abi_guard_void();
}
