#ifndef SCID_LIBSCID_HANDLES_DATABASE_SEARCH_H
#define SCID_LIBSCID_HANDLES_DATABASE_SEARCH_H

#include "scid/libscid/handles/position.h"

#include <cstddef>
#include <optional>
#include <string>

struct scid_search_header_criteria
{
        std::string player;
        std::string white;
        std::string black;
        std::string event;
        std::string site;
        std::string site_country;
        std::string round;
        std::string result;

        std::string date_min;
        std::string date_max;
        std::string event_date_min;
        std::string event_date_max;

        std::string eco_min;
        std::string eco_max;

        size_t game_number_min = 0;
        size_t game_number_max = 0;
        size_t halfmove_count_min = 0;
        size_t halfmove_count_max = 0;

        size_t white_elo_min = 0;
        size_t white_elo_max = 0;
        size_t black_elo_min = 0;
        size_t black_elo_max = 0;
        int    elo_difference_min = 0;
        int    elo_difference_max = 0;

        bool has_variations = false;
        bool has_comments = false;
        bool has_nags = false;
};

struct scid_search_board_criteria
{
        std::optional<scid::core::Position> position;
        int                                 match = 0;
        bool                                include_variations = false;
        bool                                include_flipped = false;
};

#endif
