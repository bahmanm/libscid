#pragma once

#include "bytebuf.h"
#include "scid/core/position.h"
#include "scid/database/common.h"
#include "scid/database/indexentry.h"
#include "scid/database/namebase.h"

#include <cstddef>
#include <utility>
#include <vector>

namespace scid::core
{
    class Game;
} // namespace scid::core

namespace scid::database::game_storage
{

    std::pair<IndexEntry, TagRoster>
    encode(
        const scid::core::Game& game,
        const char* scidFlags,
        std::vector<scid::core::byte>& dest);
    void
    loadStandardTags(
        scid::core::Game& game,
        char* scidFlags,
        std::size_t scidFlagsLen,
        IndexEntry const& ie,
        TagRoster const& tags);
    scid::core::errorT
    decode(
        scid::core::Game& game,
        char* scidFlags,
        std::size_t scidFlagsLen,
        IndexEntry const& ie,
        TagRoster const& tags,
        ByteBuffer buf);
    scid::core::errorT
    decodeMovesOnly(scid::core::Game& game, ByteBuffer& buf);

    scid::core::errorT
    decodeEncodedMove(
        ByteBuffer& buf,
        scid::core::byte val,
        const scid::core::Position& pos,
        scid::core::MoveSpec& action);
    scid::core::errorT
    decodeMainlineMove(
        ByteBuffer& buf,
        const scid::core::Position& pos,
        scid::core::MoveSpec& action);

} // namespace scid::database::game_storage
