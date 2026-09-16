#ifndef LIBSCID_TEST_FAULT_INJECTOR_H
#define LIBSCID_TEST_FAULT_INJECTOR_H

#include <cstddef>

namespace scid::test
{

    void
    enable_allocation_failure(std::size_t fail_at_count = 1) noexcept;

    void
    disable_allocation_failure() noexcept;

    bool
    allocation_failure_active() noexcept;

} // namespace scid::test

#endif // LIBSCID_TEST_FAULT_INJECTOR_H
