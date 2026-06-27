#include <scid/core/date.h>
#include <scid/core/error.h>
#include <scid/database/misc.h>
#include <scid/database/namebase.h>
#include <scid/spelling/spelling.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace
{

    std::string
    date_to_string(
        scid::core::dateT date)
    {
        char text[16] = {};
        scid::core::date_DecodeToString(date, text);
        return text;
    }

} // namespace

int
main()
{
    const auto [err, spelling] = scid::spelling::SpellChecker::create(
        LIBSCID_EXAMPLE_SPELLING_FILE, scid::database::Progress());

    if (err != scid::core::OK)
    {
        std::cerr << "could not load spelling file, error " << err << '\n';
        return 1;
    }

    const auto matches = spelling->find(scid::database::NAME_PLAYER, "Emanuel Lasker");

    if (matches.empty())
    {
        return 1;
    }

    std::vector<const char*> bio;
    const auto* info = spelling->getPlayerInfo(matches.front(), &bio);
    if (info == nullptr || bio.empty())
    {
        return 1;
    }

    std::string event = "Example New York";
    spelling->getGeneralCorrections(scid::database::NAME_EVENT).normalize(&event);

    std::cout << "Input: Emanuel Lasker\n"
              << "Corrected: " << matches.front() << '\n'
              << "Title: " << info->getTitle() << '\n'
              << "Country: " << info->getLastCountry() << '\n'
              << "Birth: " << date_to_string(info->getBirthdate()) << '\n'
              << "Event: " << event << '\n'
              << "Bio: " << bio.front() << '\n';

    if (std::string(matches.front()) != "Lasker, Emanuel")
    {
        return 1;
    }
    if (std::string(info->getTitle()) != "GM")
    {
        return 1;
    }
    if (info->getLastCountry() != "GER")
    {
        return 1;
    }
    if (date_to_string(info->getBirthdate()) != "1868.12.24")
    {
        return 1;
    }
    if (event != "Example, New York")
    {
        return 1;
    }

    return 0;
}
