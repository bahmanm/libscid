#include "fault_injector.h"

#include <cstdlib>
#include <new>

namespace scid::test
{

    static bool        g_fault_active = false;
    static std::size_t g_alloc_count = 0;
    static std::size_t g_fail_at = 0;

    void
    enable_allocation_failure(std::size_t fail_at_count) noexcept
    {
        g_fault_active = true;
        g_alloc_count = 0;
        g_fail_at = fail_at_count;
    }

    void
    disable_allocation_failure() noexcept
    {
        g_fault_active = false;
        g_alloc_count = 0;
        g_fail_at = 0;
    }

    bool
    allocation_failure_active() noexcept
    {
        return g_fault_active;
    }

    static void
    check_and_trigger_fault()
    {
        if (g_fault_active)
        {
            g_alloc_count++;
            if (g_alloc_count >= g_fail_at)
            {
                g_fault_active = false;
                throw std::bad_alloc();
            }
        }
    }

} // namespace scid::test

void*
operator new(std::size_t size)
{
    scid::test::check_and_trigger_fault();
    void* p = std::malloc(size);
    if (!p)
    {
        throw std::bad_alloc();
    }
    return p;
}

void*
operator new[](std::size_t size)
{
    scid::test::check_and_trigger_fault();
    void* p = std::malloc(size);
    if (!p)
    {
        throw std::bad_alloc();
    }
    return p;
}

void
operator delete(void* p) noexcept
{
    std::free(p);
}

void
operator delete[](void* p) noexcept
{
    std::free(p);
}

void
operator delete(
    void* p,
    std::size_t) noexcept
{
    std::free(p);
}

void
operator delete[](
    void* p,
    std::size_t) noexcept
{
    std::free(p);
}
