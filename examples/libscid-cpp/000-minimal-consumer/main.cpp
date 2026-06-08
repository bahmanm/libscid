#include <scid/core/position.h>

#include <iostream>

int main()
{
    const auto& position = scid::core::Position::getStdStart();
    std::cout << "libscid is available\n";

    return position.IsStdStart() ? 0 : 1;
}
