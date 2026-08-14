#ifndef SCID_LIBSCID_SUPPORT_H
#define SCID_LIBSCID_SUPPORT_H

#include "scid/movespec.h"
#include "scid/primitives.h"

#include "scid/core/game.h"
#include "scid/core/movetext_cursor.h"
#include "scid/core/position.h"
#include "scid/database/scidbase.h"

#include <cstddef>
#include <string>
#include <string_view>

struct scid_database;
struct scid_game;
struct scid_game_cursor;
struct scid_position;

namespace scid::libscid
{

    bool
    square_is_valid(scid_square square);

    scid_error
    parse_square_chars(
        char         file,
        char         rank_text,
        scid_square* out_square);

    scid_error
    parse_square(
        const char*  text,
        scid_square* out_square);

    bool
    promotion_is_valid(scid_piece promotion);

    char
    promotion_to_char(scid_piece promotion);

    scid_error
    promotion_from_char(
        char        text,
        scid_piece* out_piece);

    scid_error
    write_text(
        std::string_view text,
        char*            out_text,
        size_t           out_text_capacity,
        size_t*          out_text_size);

    scid_error
    write_bool(
        bool value,
        int* out_value);

    scid_error
    write_size(
        size_t  value,
        size_t* out_value);

    scid_piece
    piece_to_c(scid::core::pieceT piece);

    scid_error
    movespec_to_core(
        scid_movespec         move,
        scid::core::MoveSpec* out_move);

    scid_movespec
    movespec_from_core(const scid::core::MoveSpec& move);

    scid_error
    write_move_spec(
        const scid::core::Move* move,
        scid_movespec*          out_move);

    scid_error
    write_move_comment(
        const scid::core::Move* move,
        char*                   out_text,
        size_t                  out_text_capacity,
        size_t*                 out_text_size);

    scid_error
    write_move_nag_count(
        const scid::core::Move* move,
        size_t*                 out_count);

    scid_error
    write_move_nag_at(
        const scid::core::Move* move,
        size_t                  index,
        scid_nag*               out_nag);

    bool
    nag_is_move_annotation(scid::core::Nag nag);

    bool
    nag_is_position_annotation(scid::core::Nag nag);

    scid_error
    validate_move_at_cursor(
        const scid_game_cursor*     cursor,
        const scid::core::MoveSpec& move);

    scid_error
    validate_cursor_game(
        scid_game*              game,
        const scid_game_cursor* cursor);

    scid_error
    create_cursor_at(
        scid_game*                          game,
        const scid::core::MovetextLocation& location,
        scid_game_cursor**                  out_cursor);

    scid_error
    create_cursor_copy(
        const scid_game_cursor* source_cursor,
        scid_game_cursor**      out_cursor);

    std::string_view
    eco_name_from_line(std::string_view line);

    std::string
    date_to_string(scid::core::dateT date);

    bool
    database_game_index_to_core(
        size_t                    index,
        scid::database::gamenumT* out_index);

    bool
    database_game_info_get(
        const scid::database::scidBaseT& database,
        size_t                           index,
        scid::database::GameInfo*        out_info);

    bool
    database_game_index_is_valid(
        const scid::database::scidBaseT& database,
        size_t                           index,
        scid::database::gamenumT*        out_index);

    scid_error
    database_error_to_c(scid::core::errorT error);

    bool
    filter_value_is_valid(unsigned value);

    bool
    database_filter_id_is_builtin(std::string_view filter_id);

    bool
    database_filter_id_is_mutable(std::string_view filter_id);

    bool
    database_filter_get(
        const scid_database*     database,
        const char*              filter_id,
        scid::database::HFilter* out_filter);

    scid_error
    database_open(
        std::string_view                db_type,
        scid::database::fileModeT       mode,
        const char*                     path,
        scid_database**                 out_database,
        const scid::database::Progress* progress = nullptr);

    scid_error
    result_from_string(
        std::string_view     text,
        scid::core::resultT* out_result);

    std::string
    position_fen(const scid::core::Position& position);

    std::string
    game_tag_value(
        const scid::core::Game& game,
        std::string_view        name);

    bool
    game_has_special_tag(
        const scid::core::Game& game,
        std::string_view        name);

    size_t
    game_tag_count(const scid::core::Game& game);

    bool
    game_tag_at(
        const scid::core::Game& game,
        size_t                  index,
        std::string_view*       out_name,
        std::string*            out_value);

    std::string
    database_game_tag_value(
        const scid::database::scidBaseT& database,
        scid::database::gamenumT         index,
        std::string_view                 name);

    scid_error
    game_set_tag(
        scid::core::Game& game,
        std::string_view  name,
        std::string_view  value);

    scid_error
    write_optional_diagnostic(
        std::string_view text,
        char*            out_text,
        size_t           out_text_capacity,
        size_t*          out_text_size);

    scid_error
    write_optional_text(
        std::string_view text,
        char*            out_text,
        size_t           out_text_capacity,
        size_t*          out_text_size);

    scid_error
    write_position(
        const scid::core::Position& source,
        scid_position*              out_position);

    scid::core::Position
    game_start_position(const scid::core::Game& game);

    bool
    positions_match(
        const scid::core::Position& lhs,
        const scid::core::Position& rhs);

    scid_error
    validate_move_sequence(
        const scid::core::MoveSequence& sequence,
        const scid::core::Position&     start_position);

    scid_error
    append_move_sequence(
        scid::core::MovetextCursor&     cursor,
        const scid::core::MoveSequence& sequence);

    scid_error
    maybe_set_line_start_comment(
        scid::core::MovetextCursor& cursor,
        std::string_view            comment);

}

#endif
