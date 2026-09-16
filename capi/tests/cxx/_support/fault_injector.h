#ifndef LIBSCID_TEST_FAULT_INJECTOR_H
#define LIBSCID_TEST_FAULT_INJECTOR_H

#include "test_libscid.h"
#include "scid/primitives.h"

#include <cassert>
#include <cstddef>
#include <cstdint>

namespace scid::test
{

    void
    enable_allocation_failure(std::size_t fail_at_count = 1) noexcept;

    void
    disable_allocation_failure() noexcept;

    bool
    allocation_failure_active() noexcept;

    std::size_t
    allocation_count() noexcept;

    template <typename T>
    inline T*
    dirty_pointer() noexcept
    {
        return reinterpret_cast<T*>(static_cast<std::uintptr_t>(0xdeadbeef));
    }

    template <typename Fn>
    void
    assert_allocation_resilience(
        Fn&&        operation,
        std::size_t max_attempts = 500)
    {
        std::size_t fail_count = 1;
        while (fail_count < max_attempts)
        {
            enable_allocation_failure(fail_count);
            const scid_error res = operation();
            const bool       active = allocation_failure_active();
            disable_allocation_failure();

            if (active && res == SCID_OK)
            {
                break;
            }

            assert(res == SCID_ERROR_NO_MEMORY);
            fail_count++;
        }
        assert(fail_count < max_attempts);
    }

} // namespace scid::test

#endif // LIBSCID_TEST_FAULT_INJECTOR_H
