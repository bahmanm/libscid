#include "scid/scid.h"

#include "scid/core/date.h"
#include "scid/core/game.h"
#include "scid/core/game_cursor.h"
#include "scid/core/game_result.h"
#include "scid/core/movetext_cursor.h"
#include "scid/core/nags.h"
#include "scid/core/notation.h"
#include "scid/core/pgn/decode.h"
#include "scid/core/pgn/encode.h"
#include "scid/core/position.h"
#include "scid/core/primitives.h"
#include "scid/database/scidbase.h"
#include "scid/eco/book.h"
#include "scid/eco/code.h"

#include <array>
#include <cctype>
#include <cstring>
#include <limits>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

struct scid_position {
    scid::core::Position value;
};

struct scid_game {
    scid::core::Game value;
};

struct scid_movetext_cursor {
    scid_game* game = nullptr;
    scid::core::MovetextCursor value;

    explicit scid_movetext_cursor(scid_game* source_game)
        : game(source_game), value(source_game->value) {}
};

struct scid_eco_book {
    scid::eco::Book value;
};

struct scid_database {
    scid::database::scidBaseT value;
    std::string type;
};

namespace {

constexpr std::string_view pgn_roster_tags[] = {
    "Event",
    "Site",
    "Date",
    "Round",
    "White",
    "Black",
    "Result",
};

constexpr size_t pgn_roster_tag_count =
    sizeof(pgn_roster_tags) / sizeof(pgn_roster_tags[0]);

constexpr std::string_view pgn_special_tags[] = {
    "ECO",
    "EventDate",
};

bool square_is_valid(scid_square square) {
    return square <= scid::core::H8;
}

scid_error parse_square_chars(
    char file,
    char rank_text,
    scid_square* out_square
) {
    if (out_square == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    if (file >= 'A' && file <= 'H') {
        file = static_cast<char>(file - 'A' + 'a');
    }

    const auto fyle = scid::core::fyle_FromChar(file);
    const auto rank = scid::core::rank_FromChar(rank_text);
    if (fyle == scid::core::NO_FYLE || rank == scid::core::NO_RANK) {
        return SCID_ERROR_BAD_ARG;
    }

    *out_square = scid::core::square_Make(fyle, rank);
    return SCID_OK;
}

scid_error parse_square(
    const char* text,
    scid_square* out_square
) {
    if (text == nullptr || out_square == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    if (text[0] == '\0' || text[1] == '\0' || text[2] != '\0') {
        return SCID_ERROR_BAD_ARG;
    }

    return parse_square_chars(text[0], text[1], out_square);
}

bool promotion_is_valid(scid_piece promotion) {
    return promotion == SCID_PIECE_NONE ||
           promotion == SCID_PIECE_QUEEN ||
           promotion == SCID_PIECE_ROOK ||
           promotion == SCID_PIECE_BISHOP ||
           promotion == SCID_PIECE_KNIGHT;
}

char promotion_to_char(scid_piece promotion) {
    switch (promotion) {
    case SCID_PIECE_QUEEN:
        return 'q';
    case SCID_PIECE_ROOK:
        return 'r';
    case SCID_PIECE_BISHOP:
        return 'b';
    case SCID_PIECE_KNIGHT:
        return 'n';
    default:
        return '\0';
    }
}

scid_error promotion_from_char(
    char text,
    scid_piece* out_piece
) {
    if (out_piece == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    switch (text) {
    case 'Q':
    case 'q':
        *out_piece = SCID_PIECE_QUEEN;
        return SCID_OK;
    case 'R':
    case 'r':
        *out_piece = SCID_PIECE_ROOK;
        return SCID_OK;
    case 'B':
    case 'b':
        *out_piece = SCID_PIECE_BISHOP;
        return SCID_OK;
    case 'N':
    case 'n':
        *out_piece = SCID_PIECE_KNIGHT;
        return SCID_OK;
    default:
        return SCID_ERROR_BAD_ARG;
    }
}

scid_error write_text(
    std::string_view text,
    char* out_text,
    size_t out_text_capacity,
    size_t* out_text_size
) {
    if (out_text_size == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    *out_text_size = text.size();

    if (text.empty()) {
        if (out_text != nullptr && out_text_capacity > 0) {
            out_text[0] = '\0';
        }

        return SCID_OK;
    }

    if (out_text == nullptr || out_text_capacity <= text.size()) {
        return SCID_ERROR_BUFFER_FULL;
    }

    std::memcpy(out_text, text.data(), text.size());
    out_text[text.size()] = '\0';
    return SCID_OK;
}

scid_error write_bool(
    bool value,
    int* out_value
) {
    if (out_value == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    *out_value = value ? 1 : 0;
    return SCID_OK;
}

scid_error write_size(
    size_t value,
    size_t* out_value
) {
    if (out_value == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    *out_value = value;
    return SCID_OK;
}

scid_piece piece_to_c(scid::core::pieceT piece) {
    return piece == scid::core::EMPTY ? SCID_PIECE_NONE : piece;
}

scid_error movespec_to_core(
    scid_movespec move,
    scid::core::MoveSpec* out_move
) {
    if (out_move == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    if (!promotion_is_valid(move.promotion)) {
        return SCID_ERROR_BAD_ARG;
    }

    if (!square_is_valid(move.from) || !square_is_valid(move.to)) {
        return SCID_ERROR_BAD_ARG;
    }

    out_move->from = static_cast<scid::core::squareT>(move.from);
    out_move->to = static_cast<scid::core::squareT>(move.to);
    out_move->promotion = move.promotion == SCID_PIECE_NONE
                              ? scid::core::EMPTY
                              : static_cast<scid::core::pieceT>(move.promotion);
    out_move->castling = move.is_castling != 0;
    return SCID_OK;
}

scid_movespec movespec_from_core(
    const scid::core::MoveSpec& move
) {
    return {
        move.from,
        move.to,
        move.promotion == scid::core::EMPTY
            ? SCID_PIECE_NONE
            : static_cast<scid_piece>(move.promotion),
        move.castling ? 1 : 0
    };
}

scid_error write_move_spec(
    const scid::core::Move* move,
    scid_movespec* out_move
) {
    if (out_move == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    if (move == nullptr) {
        return SCID_ERROR_INVALID_MOVE;
    }

    *out_move = movespec_from_core(move->spec);
    return SCID_OK;
}

scid_error write_move_comment(
    const scid::core::Move* move,
    char* out_text,
    size_t out_text_capacity,
    size_t* out_text_size
) {
    if (out_text_size == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    if (move == nullptr) {
        return SCID_ERROR_INVALID_MOVE;
    }

    return write_text(move->metadata.comment, out_text, out_text_capacity, out_text_size);
}

scid_error write_move_nag_count(
    const scid::core::Move* move,
    size_t* out_count
) {
    if (out_count == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    if (move == nullptr) {
        return SCID_ERROR_INVALID_MOVE;
    }

    return write_size(move->metadata.nags.size(), out_count);
}

scid_error write_move_nag_at(
    const scid::core::Move* move,
    size_t index,
    scid_nag* out_nag
) {
    if (out_nag == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    if (move == nullptr) {
        return SCID_ERROR_INVALID_MOVE;
    }

    if (index >= move->metadata.nags.size()) {
        return SCID_ERROR_BAD_ARG;
    }

    *out_nag = scid::core::nagCode(move->metadata.nags[index]);
    return SCID_OK;
}

bool nag_is_move_annotation(
    scid::core::Nag nag
) {
    const auto value = scid::core::nagCode(nag);
    return value >= 1 && value <= 6;
}

bool nag_is_position_annotation(
    scid::core::Nag nag
) {
    const auto value = scid::core::nagCode(nag);
    return value >= 10 && value <= 21;
}

scid_error validate_move_at_cursor(
    const scid_movetext_cursor* cursor,
    const scid::core::MoveSpec& move
) {
    scid::core::GameCursor read_cursor(cursor->game->value);
    if (!read_cursor.restore(cursor->value.location())) {
        return SCID_ERROR;
    }

    auto position = read_cursor.currentPosition();
    if (!position) {
        return SCID_ERROR_INVALID_MOVE;
    }

    return position->applyMove(move);
}

std::string_view eco_name_from_line(
    std::string_view line
) {
    const auto name_start = line.find('[');
    const auto name_end = line.rfind(']');
    if (name_start == std::string_view::npos ||
        name_end == std::string_view::npos ||
        name_start >= name_end) {
        return {};
    }

    return line.substr(name_start + 1, name_end - name_start - 1);
}

std::string date_to_string(
    scid::core::dateT date
) {
    char text[16] = {};
    scid::core::date_DecodeToString(date, text);
    return text;
}

bool database_game_index_to_core(
    size_t index,
    scid::database::gamenumT* out_index
) {
    if (out_index == nullptr ||
        index > std::numeric_limits<scid::database::gamenumT>::max()) {
        return false;
    }

    *out_index = static_cast<scid::database::gamenumT>(index);
    return true;
}

bool database_game_info_get(
    const scid::database::scidBaseT& database,
    size_t index,
    scid::database::GameInfo* out_info
) {
    if (out_info == nullptr) {
        return false;
    }

    scid::database::gamenumT game_index = 0;
    if (!database_game_index_to_core(index, &game_index)) {
        return false;
    }

    const auto info = database.gameInfoBounds(game_index);
    if (!info) {
        return false;
    }

    *out_info = *info;
    return true;
}

bool database_game_index_is_valid(
    const scid::database::scidBaseT& database,
    size_t index,
    scid::database::gamenumT* out_index
) {
    scid::database::gamenumT game_index = 0;
    if (!database_game_index_to_core(index, &game_index) ||
        !database.gameInfoBounds(game_index)) {
        return false;
    }

    if (out_index != nullptr) {
        *out_index = game_index;
    }

    return true;
}

scid_error database_error_to_c(
    scid::core::errorT error
) {
    switch (error) {
    case scid::core::OK:
        return SCID_OK;
    case scid::core::ERROR_BadArg:
        return SCID_ERROR_BAD_ARG;
    case scid::core::ERROR_FileOpen:
        return SCID_ERROR_FILE_OPEN;
    case scid::core::ERROR_FileReadOnly:
        return SCID_ERROR_FILE_READ_ONLY;
    case scid::core::ERROR_Corrupt:
        return SCID_ERROR_CORRUPT;
    default:
        return SCID_ERROR;
    }
}

bool filter_value_is_valid(
    unsigned value
) {
    return value <= std::numeric_limits<scid::core::byte>::max();
}

bool database_filter_id_is_builtin(
    std::string_view filter_id
) {
    return filter_id == "all" || filter_id == "dbfilter";
}

bool database_filter_id_is_mutable(
    std::string_view filter_id
) {
    return filter_id != "all";
}

bool database_filter_get(
    const scid_database* database,
    const char* filter_id,
    scid::database::HFilter* out_filter
) {
    if (database == nullptr ||
        filter_id == nullptr ||
        out_filter == nullptr ||
        !database->value.isOpen()) {
        return false;
    }

    auto filter = database->value.getFilter(filter_id);
    if (filter == nullptr) {
        return false;
    }

    *out_filter = filter;
    return true;
}

scid_error database_open(
    std::string_view db_type,
    scid::database::fileModeT mode,
    const char* path,
    scid_database** out_database
) {
    if (path == nullptr || out_database == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        auto* database = new scid_database;
        const auto error = database->value.open(db_type, mode, path);
        if (error != scid::core::OK) {
            delete database;
            *out_database = nullptr;
            return database_error_to_c(error);
        }

        if (db_type == "MEMORY") {
            database->type = "memory";
        } else if (db_type == "SCID5") {
            database->type = "scid5";
        } else {
            database->type.assign(db_type);
        }

        *out_database = database;
        return SCID_OK;
    } catch (...) {
        *out_database = nullptr;
        return SCID_ERROR;
    }
}

scid_error result_from_string(
    std::string_view text,
    scid::core::resultT* out_result
) {
    if (out_result == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    if (text == "*") {
        *out_result = scid::core::RESULT_None;
        return SCID_OK;
    }

    if (text == "1-0") {
        *out_result = scid::core::RESULT_White;
        return SCID_OK;
    }

    if (text == "0-1") {
        *out_result = scid::core::RESULT_Black;
        return SCID_OK;
    }

    if (text == "1/2-1/2") {
        *out_result = scid::core::RESULT_Draw;
        return SCID_OK;
    }

    return SCID_ERROR_BAD_ARG;
}

std::string position_fen(
    const scid::core::Position& position
) {
    char fen[256];
    position.PrintFEN(fen, sizeof(fen));
    return fen;
}

std::string game_tag_value(
    const scid::core::Game& game,
    std::string_view name
) {
    if (name == "Event") {
        return game.event();
    }
    if (name == "Site") {
        return game.site();
    }
    if (name == "Date") {
        return date_to_string(game.date());
    }
    if (name == "Round") {
        return game.round();
    }
    if (name == "White") {
        return game.white().name;
    }
    if (name == "Black") {
        return game.black().name;
    }
    if (name == "Result") {
        return std::string(game.resultString());
    }
    if (name == "ECO") {
        return game.eco();
    }
    if (name == "EventDate") {
        if (game.eventDate() == scid::core::ZERO_DATE) {
            return {};
        }
        return date_to_string(game.eventDate());
    }
    if (name == "FEN") {
        if (const scid::core::Position* position = game.startPosition()) {
            return position_fen(*position);
        }
        return {};
    }

    if (const std::string* value = game.findExtraTag(name)) {
        return *value;
    }

    return {};
}

bool game_has_special_tag(
    const scid::core::Game& game,
    std::string_view name
) {
    if (name == "ECO") {
        return !game.eco().empty();
    }
    if (name == "EventDate") {
        return game.eventDate() != scid::core::ZERO_DATE;
    }
    if (name == "FEN") {
        return game.startPosition() != nullptr;
    }

    return false;
}

size_t game_tag_count(
    const scid::core::Game& game
) {
    size_t count = pgn_roster_tag_count + game.extraTags().size();
    for (const auto tag : pgn_special_tags) {
        if (game_has_special_tag(game, tag)) {
            ++count;
        }
    }
    if (game_has_special_tag(game, "FEN")) {
        ++count;
    }

    return count;
}

bool game_tag_at(
    const scid::core::Game& game,
    size_t index,
    std::string_view* out_name,
    std::string* out_value
) {
    if (out_name == nullptr || out_value == nullptr) {
        return false;
    }

    if (index < pgn_roster_tag_count) {
        *out_name = pgn_roster_tags[index];
        *out_value = game_tag_value(game, *out_name);
        return true;
    }

    index -= pgn_roster_tag_count;

    for (const auto tag : pgn_special_tags) {
        if (!game_has_special_tag(game, tag)) {
            continue;
        }

        if (index == 0) {
            *out_name = tag;
            *out_value = game_tag_value(game, tag);
            return true;
        }

        --index;
    }

    const auto& extra_tags = game.extraTags();
    if (index < extra_tags.size()) {
        *out_name = extra_tags[index].first;
        *out_value = extra_tags[index].second;
        return true;
    }

    index -= extra_tags.size();
    if (game_has_special_tag(game, "FEN") && index == 0) {
        *out_name = "FEN";
        *out_value = game_tag_value(game, "FEN");
        return true;
    }

    return false;
}

std::string database_game_tag_value(
    const scid::database::scidBaseT& database,
    scid::database::gamenumT index,
    std::string_view name
) {
    const auto info = database.gameInfo(index);
    const auto tags = database.tagRoster(index);

    if (name == "Event") {
        return tags.event;
    }
    if (name == "Site") {
        return tags.site;
    }
    if (name == "Date") {
        return date_to_string(info.date);
    }
    if (name == "Round") {
        return tags.round;
    }
    if (name == "White") {
        return tags.white;
    }
    if (name == "Black") {
        return tags.black;
    }
    if (name == "Result") {
        return scid::core::RESULT_LONGSTR[info.result];
    }
    if (name == "ECO") {
        scid::eco::String text = {};
        scid::eco::toExtendedString(info.ecoCode, text);
        return text;
    }
    if (name == "EventDate") {
        return date_to_string(info.eventDate);
    }

    return {};
}

scid_error game_set_tag(
    scid::core::Game& game,
    std::string_view name,
    std::string_view value
) {
    if (name == "Date") {
        game.setDate(scid::core::date_parsePGNTag(value.data(), value.size()));
        return SCID_OK;
    }
    if (name == "EventDate") {
        game.setEventDate(scid::core::date_parsePGNTag(value.data(), value.size()));
        return SCID_OK;
    }
    if (name == "Result") {
        scid::core::resultT result = scid::core::RESULT_None;
        if (const scid_error error = result_from_string(value, &result);
            error != SCID_OK) {
            return error;
        }
        game.setResult(result);
        return SCID_OK;
    }
    if (name == "ECO") {
        game.setEco(value);
        return SCID_OK;
    }

    game.addTag(name, value);
    return SCID_OK;
}

scid_error write_optional_diagnostic(
    std::string_view text,
    char* out_text,
    size_t out_text_capacity,
    size_t* out_text_size
) {
    if (out_text == nullptr && out_text_capacity == 0 && out_text_size == nullptr) {
        return SCID_OK;
    }

    return write_text(text, out_text, out_text_capacity, out_text_size);
}

scid_error write_optional_text(
    std::string_view text,
    char* out_text,
    size_t out_text_capacity,
    size_t* out_text_size
) {
    if (out_text == nullptr && out_text_capacity == 0 && out_text_size == nullptr) {
        return SCID_OK;
    }

    return write_text(text, out_text, out_text_capacity, out_text_size);
}

scid_error write_position(
    const scid::core::Position& source,
    scid_position* out_position
) {
    if (out_position == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    out_position->value = source;
    return SCID_OK;
}

}

scid_error scid_square_from_string(
    const char* text,
    scid_square* out_square
) {
    return parse_square(text, out_square);
}

scid_error scid_square_to_string(
    scid_square square,
    char* out_text,
    size_t out_text_capacity,
    size_t* out_text_size
) {
    if (!square_is_valid(square)) {
        return SCID_ERROR_BAD_ARG;
    }

    char text[] = {
        static_cast<char>('a' + scid::core::square_Fyle(square)),
        static_cast<char>('1' + scid::core::square_Rank(square)),
        '\0'
    };

    return write_text(text, out_text, out_text_capacity, out_text_size);
}

scid_error scid_piece_type_from_string(
    const char* text,
    scid_piece* out_piece
) {
    if (text == nullptr || out_piece == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    if (text[0] == '\0' || text[1] != '\0') {
        return SCID_ERROR_BAD_ARG;
    }

    switch (text[0]) {
    case 'K':
    case 'k':
        *out_piece = scid::core::KING;
        return SCID_OK;
    case 'Q':
    case 'q':
        *out_piece = scid::core::QUEEN;
        return SCID_OK;
    case 'R':
    case 'r':
        *out_piece = scid::core::ROOK;
        return SCID_OK;
    case 'B':
    case 'b':
        *out_piece = scid::core::BISHOP;
        return SCID_OK;
    case 'N':
    case 'n':
        *out_piece = scid::core::KNIGHT;
        return SCID_OK;
    case 'P':
    case 'p':
        *out_piece = scid::core::PAWN;
        return SCID_OK;
    default:
        return SCID_ERROR_BAD_ARG;
    }
}

scid_error scid_movespec_create(
    scid_square from,
    scid_square to,
    scid_piece promotion,
    int is_castling,
    scid_movespec* out_move
) {
    if (out_move == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    *out_move = {
        from,
        to,
        promotion,
        is_castling != 0
    };

    return SCID_OK;
}

scid_error scid_movespec_create_from_uci(
    const char* text,
    scid_movespec* out_move
) {
    if (text == nullptr || out_move == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    if (std::strcmp(text, "0000") == 0) {
        *out_move = {
            0,
            0,
            SCID_PIECE_NONE,
            0
        };
        return SCID_OK;
    }

    const bool has_promotion =
        text[0] != '\0' &&
        text[1] != '\0' &&
        text[2] != '\0' &&
        text[3] != '\0' &&
        text[4] != '\0' &&
        text[5] == '\0';

    const bool is_quiet =
        text[0] != '\0' &&
        text[1] != '\0' &&
        text[2] != '\0' &&
        text[3] != '\0' &&
        text[4] == '\0';

    if (!is_quiet && !has_promotion) {
        return SCID_ERROR_BAD_ARG;
    }

    scid_square from = 0;
    scid_square to = 0;
    scid_piece promotion = SCID_PIECE_NONE;

    if (parse_square_chars(text[0], text[1], &from) != SCID_OK ||
        parse_square_chars(text[2], text[3], &to) != SCID_OK) {
        return SCID_ERROR_BAD_ARG;
    }

    if (has_promotion && promotion_from_char(text[4], &promotion) != SCID_OK) {
        return SCID_ERROR_BAD_ARG;
    }

    *out_move = {
        from,
        to,
        promotion,
        0
    };
    return SCID_OK;
}

scid_error scid_movespec_to_uci(
    scid_movespec move,
    char* out_text,
    size_t out_text_capacity,
    size_t* out_text_size
) {
    if (!promotion_is_valid(move.promotion)) {
        return SCID_ERROR_BAD_ARG;
    }

    if (move.from == move.to && move.is_castling == 0) {
        return write_text("0000", out_text, out_text_capacity, out_text_size);
    }

    if (!square_is_valid(move.from) || !square_is_valid(move.to)) {
        return SCID_ERROR_BAD_ARG;
    }

    char text[6] = {
        static_cast<char>('a' + scid::core::square_Fyle(move.from)),
        static_cast<char>('1' + scid::core::square_Rank(move.from)),
        static_cast<char>('a' + scid::core::square_Fyle(move.to)),
        static_cast<char>('1' + scid::core::square_Rank(move.to)),
        '\0',
        '\0'
    };

    if (move.promotion != SCID_PIECE_NONE) {
        text[4] = promotion_to_char(move.promotion);
    }

    return write_text(text, out_text, out_text_capacity, out_text_size);
}

scid_error scid_movespec_create_from_san(
    const scid_position* position,
    const char* text,
    scid_movespec* out_move
) {
    if (position == nullptr || text == nullptr || out_move == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        scid::core::MoveSpec move;
        const scid_error error =
            const_cast<scid::core::Position&>(position->value).parseMoveSpec(move, text);
        if (error != SCID_OK) {
            return error;
        }

        *out_move = movespec_from_core(move);
        return SCID_OK;
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_movespec_to_san(
    const scid_position* position,
    scid_movespec move,
    char* out_text,
    size_t out_text_capacity,
    size_t* out_text_size
) {
    if (position == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        scid::core::MoveSpec core_move;
        if (const scid_error error = movespec_to_core(move, &core_move);
            error != SCID_OK) {
            return error;
        }

        const std::string text =
            const_cast<scid::core::Position&>(position->value)
                .makeSan(core_move, scid::core::SAN_MATETEST);
        if (text.empty()) {
            return SCID_ERROR_INVALID_MOVE;
        }

        return write_text(text, out_text, out_text_capacity, out_text_size);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_nag_create_from_string(
    const char* text,
    scid_nag* out_nag
) {
    if (text == nullptr || out_nag == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        *out_nag = scid::core::nagCode(scid::core::nagFromString(text));
        return SCID_OK;
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_nag_to_string(
    scid_nag nag,
    int as_symbol,
    char* out_text,
    size_t out_text_capacity,
    size_t* out_text_size
) {
    try {
        const std::string text = scid::core::nagToString(
            scid::core::nagFromCode(nag),
            as_symbol != 0
        );

        return write_text(text, out_text, out_text_capacity, out_text_size);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_position_create_standard(
    scid_position** out_position
) {
    if (out_position == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        auto* position = new scid_position;
        position->value.StdStart();
        *out_position = position;
        return SCID_OK;
    } catch (...) {
        *out_position = nullptr;
        return SCID_ERROR;
    }
}

scid_error scid_position_create_empty(
    scid_position** out_position
) {
    if (out_position == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        auto* position = new scid_position;
        position->value.Clear();
        *out_position = position;
        return SCID_OK;
    } catch (...) {
        *out_position = nullptr;
        return SCID_ERROR;
    }
}

scid_error scid_position_create_from_fen(
    const char* fen,
    scid_position** out_position
) {
    if (fen == nullptr || out_position == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        auto* position = new scid_position;
        const scid_error error = position->value.ReadFromFEN(fen);
        if (error != SCID_OK) {
            delete position;
            *out_position = nullptr;
            return error;
        }

        *out_position = position;
        return SCID_OK;
    } catch (...) {
        *out_position = nullptr;
        return SCID_ERROR;
    }
}

void scid_position_free(
    scid_position* position
) {
    delete position;
}

scid_error scid_position_to_fen(
    const scid_position* position,
    char* out_fen,
    size_t out_fen_capacity,
    size_t* out_fen_size
) {
    if (position == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        char fen[256];
        position->value.PrintFEN(fen, sizeof(fen));
        return write_text(fen, out_fen, out_fen_capacity, out_fen_size);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_position_apply_san(
    scid_position* position,
    const char* san
) {
    if (position == nullptr || san == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        scid::core::MoveSpec move;
        if (const scid_error error = position->value.parseMoveSpec(move, san);
            error != SCID_OK) {
            return error;
        }

        return position->value.applyMove(move);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_position_apply_uci(
    scid_position* position,
    const char* uci
) {
    if (position == nullptr || uci == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        scid::core::MoveSpec move;
        if (const scid_error error =
                position->value.readCoordinateMoveSpec(move, uci, false);
            error != SCID_OK) {
            return error;
        }

        return position->value.applyMove(move);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_position_is_start(
    const scid_position* position,
    int* out_is_start
) {
    if (position == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    return write_bool(position->value.IsStdStart(), out_is_start);
}

scid_error scid_position_is_check(
    const scid_position* position,
    int* out_is_check
) {
    if (position == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        return write_bool(
            const_cast<scid::core::Position&>(position->value).IsKingInCheck(),
            out_is_check
        );
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_position_is_checkmate(
    const scid_position* position,
    int* out_is_checkmate
) {
    if (position == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        return write_bool(
            const_cast<scid::core::Position&>(position->value).IsKingInMate(),
            out_is_checkmate
        );
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_position_is_legal(
    const scid_position* position,
    int* out_is_legal
) {
    if (position == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        return write_bool(
            const_cast<scid::core::Position&>(position->value).IsLegal(),
            out_is_legal
        );
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_position_side_to_move_get(
    const scid_position* position,
    scid_colour* out_side_to_move
) {
    if (position == nullptr || out_side_to_move == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    *out_side_to_move = position->value.GetToMove() == scid::core::WHITE
                            ? SCID_WHITE
                            : SCID_BLACK;
    return SCID_OK;
}

scid_error scid_position_fullmove_number_get(
    const scid_position* position,
    unsigned* out_fullmove_number
) {
    if (position == nullptr || out_fullmove_number == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    *out_fullmove_number = position->value.GetFullMoveCount();
    return SCID_OK;
}

scid_error scid_position_halfmove_clock_get(
    const scid_position* position,
    unsigned* out_halfmove_clock
) {
    if (position == nullptr || out_halfmove_clock == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    *out_halfmove_clock = position->value.GetHalfMoveClock();
    return SCID_OK;
}

scid_error scid_position_piece_at_get(
    const scid_position* position,
    scid_square square,
    scid_piece* out_piece
) {
    if (position == nullptr || out_piece == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    if (!square_is_valid(square)) {
        return SCID_ERROR_BAD_ARG;
    }

    *out_piece = piece_to_c(position->value.GetPiece(square));
    return SCID_OK;
}

scid_error scid_game_create_empty(
    scid_game** out_game
) {
    if (out_game == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        *out_game = new scid_game;
        return SCID_OK;
    } catch (...) {
        *out_game = nullptr;
        return SCID_ERROR;
    }
}

scid_error scid_game_create_from_position(
    const scid_position* position,
    scid_game** out_game
) {
    if (position == nullptr || out_game == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        auto* game = new scid_game;
        if (!position->value.IsStdStart()) {
            game->value.setStartPosition(position->value);
        }

        *out_game = game;
        return SCID_OK;
    } catch (...) {
        *out_game = nullptr;
        return SCID_ERROR;
    }
}

scid_error scid_game_create_from_pgn(
    const char* pgn,
    size_t pgn_size,
    scid_game** out_game,
    char* out_diagnostic,
    size_t out_diagnostic_capacity,
    size_t* out_diagnostic_size
) {
    if (pgn == nullptr || out_game == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        auto* game = new scid_game;
        scid::core::pgn::ParseLog log;
        const bool ok = scid::core::pgn::parseGame(
            pgn,
            pgn_size,
            game->value,
            log
        );

        const scid_error diagnostic_error = write_optional_diagnostic(
            log.log,
            out_diagnostic,
            out_diagnostic_capacity,
            out_diagnostic_size
        );
        if (diagnostic_error != SCID_OK) {
            delete game;
            *out_game = nullptr;
            return diagnostic_error;
        }

        if (!ok) {
            delete game;
            *out_game = nullptr;
            return SCID_ERROR_CORRUPT;
        }

        *out_game = game;
        return SCID_OK;
    } catch (...) {
        *out_game = nullptr;
        return SCID_ERROR;
    }
}

void scid_game_free(
    scid_game* game
) {
    delete game;
}

scid_error scid_game_to_pgn(
    const scid_game* game,
    char* out_text,
    size_t out_text_capacity,
    size_t* out_text_size
) {
    if (game == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        std::string pgn;
        scid::core::pgn::encode(game->value, pgn);
        return write_text(pgn, out_text, out_text_capacity, out_text_size);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_game_mainline_halfmove_count_get(
    const scid_game* game,
    size_t* out_count
) {
    if (game == nullptr || out_count == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        return write_size(game->value.mainlineHalfMoveCount(), out_count);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_game_initial_comment_get(
    const scid_game* game,
    char* out_text,
    size_t out_text_capacity,
    size_t* out_text_size
) {
    if (game == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        return write_text(
            game->value.initialComment(),
            out_text,
            out_text_capacity,
            out_text_size
        );
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_game_movetext_clear(
    scid_game* game
) {
    if (game == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        game->value.clearMovetext();
        return SCID_OK;
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_game_tag_get(
    const scid_game* game,
    const char* name,
    char* out_text,
    size_t out_text_capacity,
    size_t* out_text_size
) {
    if (game == nullptr || name == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        return write_text(
            game_tag_value(game->value, name),
            out_text,
            out_text_capacity,
            out_text_size
        );
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_game_tag_set(
    scid_game* game,
    const char* name,
    const char* value
) {
    if (game == nullptr || name == nullptr || value == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        return game_set_tag(game->value, name, value);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_game_tag_count_get(
    const scid_game* game,
    size_t* out_count
) {
    if (game == nullptr || out_count == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        return write_size(game_tag_count(game->value), out_count);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_game_tag_at_get(
    const scid_game* game,
    size_t index,
    char* out_name,
    size_t out_name_capacity,
    size_t* out_name_size,
    char* out_value,
    size_t out_value_capacity,
    size_t* out_value_size
) {
    if (game == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        std::string_view name;
        std::string value;
        if (!game_tag_at(game->value, index, &name, &value)) {
            return SCID_ERROR_BAD_ARG;
        }

        if (const scid_error error =
                write_text(name, out_name, out_name_capacity, out_name_size);
            error != SCID_OK) {
            return error;
        }

        return write_text(value, out_value, out_value_capacity, out_value_size);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_game_tag_remove(
    scid_game* game,
    const char* name,
    int* out_removed
) {
    if (game == nullptr || name == nullptr || out_removed == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        const std::string_view tag_name(name);
        if (tag_name == "ECO") {
            const bool found = !game->value.eco().empty();
            game->value.setEco({});
            *out_removed = found ? 1 : 0;
            return SCID_OK;
        }

        if (tag_name == "EventDate") {
            const bool found = game->value.eventDate() != scid::core::ZERO_DATE;
            game->value.setEventDate(scid::core::ZERO_DATE);
            *out_removed = found ? 1 : 0;
            return SCID_OK;
        }

        if (tag_name == "FEN") {
            *out_removed = 0;
            return SCID_OK;
        }

        const bool found = game->value.findExtraTag(name) != nullptr;
        if (found) {
            game->value.removeExtraTag(name);
        }

        *out_removed = found ? 1 : 0;
        return SCID_OK;
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_game_start_position_get(
    const scid_game* game,
    scid_position* out_position
) {
    if (game == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        if (const scid::core::Position* position = game->value.startPosition()) {
            return write_position(*position, out_position);
        }

        return write_position(scid::core::Position::getStdStart(), out_position);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_game_final_position_get(
    const scid_game* game,
    scid_position* out_position
) {
    if (game == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        scid::core::GameCursor cursor(game->value);
        cursor.toEnd();
        const auto position = cursor.currentPosition();
        if (!position) {
            return SCID_ERROR_INVALID_MOVE;
        }

        return write_position(*position, out_position);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_movetext_cursor_create(
    scid_game* game,
    scid_movetext_cursor** out_cursor
) {
    if (game == nullptr || out_cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        *out_cursor = new scid_movetext_cursor(game);
        return SCID_OK;
    } catch (...) {
        *out_cursor = nullptr;
        return SCID_ERROR;
    }
}

void scid_movetext_cursor_free(
    scid_movetext_cursor* cursor
) {
    delete cursor;
}

scid_error scid_movetext_cursor_position_get(
    const scid_movetext_cursor* cursor,
    scid_position* out_position
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        scid::core::GameCursor read_cursor(cursor->game->value);
        if (!read_cursor.restore(cursor->value.location())) {
            return SCID_ERROR;
        }

        const auto position = read_cursor.currentPosition();
        if (!position) {
            return SCID_ERROR_INVALID_MOVE;
        }

        return write_position(*position, out_position);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_movetext_cursor_ply_get(
    const scid_movetext_cursor* cursor,
    size_t* out_ply
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    return write_size(cursor->value.ply(), out_ply);
}

scid_error scid_movetext_cursor_variation_count_get(
    const scid_movetext_cursor* cursor,
    size_t* out_count
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    return write_size(cursor->value.variationCount(), out_count);
}

scid_error scid_movetext_cursor_variation_depth_get(
    const scid_movetext_cursor* cursor,
    size_t* out_depth
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    return write_size(cursor->value.variationDepth(), out_depth);
}

scid_error scid_movetext_cursor_variation_index_get(
    const scid_movetext_cursor* cursor,
    size_t* out_index
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    return write_size(cursor->value.variationIndex(), out_index);
}

scid_error scid_movetext_cursor_is_line_start(
    const scid_movetext_cursor* cursor,
    int* out_is_line_start
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    return write_bool(cursor->value.isAtLineStart(), out_is_line_start);
}

scid_error scid_movetext_cursor_is_line_end(
    const scid_movetext_cursor* cursor,
    int* out_is_line_end
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    return write_bool(cursor->value.isAtLineEnd(), out_is_line_end);
}

scid_error scid_movetext_cursor_is_game_start(
    const scid_movetext_cursor* cursor,
    int* out_is_game_start
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    return write_bool(cursor->value.isAtGameStart(), out_is_game_start);
}

scid_error scid_movetext_cursor_is_game_end(
    const scid_movetext_cursor* cursor,
    int* out_is_game_end
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    return write_bool(cursor->value.isAtGameEnd(), out_is_game_end);
}

scid_error scid_movetext_cursor_is_variation_start(
    const scid_movetext_cursor* cursor,
    int* out_is_variation_start
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    return write_bool(cursor->value.isAtVariationStart(), out_is_variation_start);
}

scid_error scid_movetext_cursor_is_variation_end(
    const scid_movetext_cursor* cursor,
    int* out_is_variation_end
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    return write_bool(cursor->value.isAtVariationEnd(), out_is_variation_end);
}

scid_error scid_movetext_cursor_is_variation_empty(
    const scid_movetext_cursor* cursor,
    int* out_is_variation_empty
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    return write_bool(cursor->value.isAtEmptyVariation(), out_is_variation_empty);
}

scid_error scid_movetext_cursor_comment_get(
    const scid_movetext_cursor* cursor,
    char* out_text,
    size_t out_text_capacity,
    size_t* out_text_size
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        if (cursor->value.isAtLineStart()) {
            if (cursor->value.variationDepth() == 0) {
                return write_text(
                    cursor->game->value.initialComment(),
                    out_text,
                    out_text_capacity,
                    out_text_size
                );
            }

            const auto* variation = cursor->value.currentVariation();
            if (variation == nullptr) {
                return SCID_ERROR;
            }

            return write_text(
                variation->initialComment,
                out_text,
                out_text_capacity,
                out_text_size
            );
        }

        return write_move_comment(
            cursor->value.previousMove(),
            out_text,
            out_text_capacity,
            out_text_size
        );
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_movetext_cursor_comment_set(
    scid_movetext_cursor* cursor,
    const char* comment
) {
    if (cursor == nullptr || comment == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        return cursor->value.setComment(comment) ? SCID_OK : SCID_ERROR;
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_movetext_cursor_previous_movespec_get(
    const scid_movetext_cursor* cursor,
    scid_movespec* out_move
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    return write_move_spec(cursor->value.previousMove(), out_move);
}

scid_error scid_movetext_cursor_previous_move_san_get(
    const scid_movetext_cursor* cursor,
    char* out_text,
    size_t out_text_capacity,
    size_t* out_text_size
) {
    if (cursor == nullptr || out_text_size == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        const auto san = scid::core::notation::previousSan(
            cursor->game->value,
            cursor->value.location()
        );
        if (san.empty()) {
            return SCID_ERROR_INVALID_MOVE;
        }

        return write_text(san, out_text, out_text_capacity, out_text_size);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_movetext_cursor_previous_move_comment_get(
    const scid_movetext_cursor* cursor,
    char* out_text,
    size_t out_text_capacity,
    size_t* out_text_size
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    return write_move_comment(
        cursor->value.previousMove(),
        out_text,
        out_text_capacity,
        out_text_size
    );
}

scid_error scid_movetext_cursor_previous_move_nag_count_get(
    const scid_movetext_cursor* cursor,
    size_t* out_count
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    return write_move_nag_count(cursor->value.previousMove(), out_count);
}

scid_error scid_movetext_cursor_previous_move_nag_at_get(
    const scid_movetext_cursor* cursor,
    size_t index,
    scid_nag* out_nag
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    return write_move_nag_at(cursor->value.previousMove(), index, out_nag);
}

scid_error scid_movetext_cursor_next_movespec_get(
    const scid_movetext_cursor* cursor,
    scid_movespec* out_move
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    return write_move_spec(cursor->value.nextMove(), out_move);
}

scid_error scid_movetext_cursor_next_move_san_get(
    const scid_movetext_cursor* cursor,
    char* out_text,
    size_t out_text_capacity,
    size_t* out_text_size
) {
    if (cursor == nullptr || out_text_size == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        const auto san = scid::core::notation::nextSan(
            cursor->game->value,
            cursor->value.location()
        );
        if (san.empty()) {
            return SCID_ERROR_INVALID_MOVE;
        }

        return write_text(san, out_text, out_text_capacity, out_text_size);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_movetext_cursor_next_move_comment_get(
    const scid_movetext_cursor* cursor,
    char* out_text,
    size_t out_text_capacity,
    size_t* out_text_size
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    return write_move_comment(
        cursor->value.nextMove(),
        out_text,
        out_text_capacity,
        out_text_size
    );
}

scid_error scid_movetext_cursor_next_move_nag_count_get(
    const scid_movetext_cursor* cursor,
    size_t* out_count
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    return write_move_nag_count(cursor->value.nextMove(), out_count);
}

scid_error scid_movetext_cursor_next_move_nag_at_get(
    const scid_movetext_cursor* cursor,
    size_t index,
    scid_nag* out_nag
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    return write_move_nag_at(cursor->value.nextMove(), index, out_nag);
}

scid_error scid_movetext_cursor_to_start(
    scid_movetext_cursor* cursor
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    cursor->value.toStart();
    return SCID_OK;
}

scid_error scid_movetext_cursor_to_end(
    scid_movetext_cursor* cursor
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    cursor->value.toEnd();
    return SCID_OK;
}

scid_error scid_movetext_cursor_to_ply(
    scid_movetext_cursor* cursor,
    size_t ply,
    int* out_moved
) {
    if (cursor == nullptr || out_moved == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    return write_bool(cursor->value.toPly(ply), out_moved);
}

scid_error scid_movetext_cursor_next(
    scid_movetext_cursor* cursor,
    int* out_moved
) {
    if (cursor == nullptr || out_moved == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    return write_bool(cursor->value.next(), out_moved);
}

scid_error scid_movetext_cursor_previous(
    scid_movetext_cursor* cursor,
    int* out_moved
) {
    if (cursor == nullptr || out_moved == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    return write_bool(cursor->value.previous(), out_moved);
}

scid_error scid_movetext_cursor_variation_enter(
    scid_movetext_cursor* cursor,
    size_t index,
    int* out_entered
) {
    if (cursor == nullptr || out_entered == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    return write_bool(cursor->value.enterVariation(index), out_entered);
}

scid_error scid_movetext_cursor_variation_exit(
    scid_movetext_cursor* cursor,
    int* out_exited
) {
    if (cursor == nullptr || out_exited == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    return write_bool(cursor->value.exitVariation(), out_exited);
}

scid_error scid_movetext_cursor_move_add(
    scid_movetext_cursor* cursor,
    scid_movespec move
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        scid::core::MoveSpec core_move;
        if (const scid_error error = movespec_to_core(move, &core_move);
            error != SCID_OK) {
            return error;
        }

        if (const scid_error error = validate_move_at_cursor(cursor, core_move);
            error != SCID_OK) {
            return error;
        }

        cursor->value.addMove(core_move);
        return SCID_OK;
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_movetext_cursor_variation_add(
    scid_movetext_cursor* cursor,
    const char* initial_comment,
    int* out_added
) {
    if (cursor == nullptr || out_added == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        const auto* comment = initial_comment == nullptr ? "" : initial_comment;
        return write_bool(cursor->value.addVariation(comment) != nullptr, out_added);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_movetext_cursor_nag_add(
    scid_movetext_cursor* cursor,
    scid_nag nag,
    int* out_added
) {
    if (cursor == nullptr || out_added == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        if (cursor->value.previousMove() == nullptr || nag == 0) {
            return write_bool(false, out_added);
        }

        return write_bool(
            cursor->value.addPreviousMoveNag(scid::core::nagFromCode(nag)),
            out_added
        );
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_movetext_cursor_nag_remove(
    scid_movetext_cursor* cursor,
    int is_move_nag,
    int* out_removed
) {
    if (cursor == nullptr || out_removed == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        auto* move = cursor->value.previousMove();
        if (move == nullptr) {
            return write_bool(false, out_removed);
        }

        auto& nags = move->metadata.nags;
        for (auto it = nags.begin(); it != nags.end(); ++it) {
            const bool matches = is_move_nag != 0
                                     ? nag_is_move_annotation(*it)
                                     : nag_is_position_annotation(*it);
            if (matches) {
                nags.erase(it);
                return write_bool(true, out_removed);
            }
        }

        return write_bool(false, out_removed);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_movetext_cursor_nag_clear(
    scid_movetext_cursor* cursor
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        cursor->value.clearPreviousMoveNags();
        return SCID_OK;
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_movetext_cursor_variation_promote_to_first(
    scid_movetext_cursor* cursor,
    int* out_promoted
) {
    if (cursor == nullptr || out_promoted == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        return write_bool(cursor->value.promoteVariationToFirst(), out_promoted);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_movetext_cursor_variation_promote_to_mainline(
    scid_movetext_cursor* cursor,
    int* out_promoted
) {
    if (cursor == nullptr || out_promoted == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        return write_bool(cursor->value.promoteVariationToMainline(), out_promoted);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_movetext_cursor_variation_delete(
    scid_movetext_cursor* cursor,
    int* out_deleted
) {
    if (cursor == nullptr || out_deleted == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        return write_bool(cursor->value.deleteVariation(), out_deleted);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_movetext_cursor_truncate(
    scid_movetext_cursor* cursor
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        cursor->value.truncate();
        return SCID_OK;
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_movetext_cursor_truncate_before_cursor(
    scid_movetext_cursor* cursor
) {
    if (cursor == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        cursor->value.truncateBeforeCursor();
        return SCID_OK;
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_eco_code_from_string(
    const char* text,
    scid_eco_code* out_code
) {
    if (text == nullptr || out_code == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        *out_code = scid::eco::fromString(text);
        return SCID_OK;
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_eco_code_to_string(
    scid_eco_code code,
    scid_eco_format format,
    char* out_text,
    size_t out_text_capacity,
    size_t* out_text_size
) {
    if (format != SCID_ECO_FORMAT_BASIC &&
        format != SCID_ECO_FORMAT_EXTENDED) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        scid::eco::String text = {};
        scid::eco::toString(
            code,
            text,
            format == SCID_ECO_FORMAT_EXTENDED
        );
        return write_text(text, out_text, out_text_capacity, out_text_size);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_eco_book_load(
    const char* path,
    scid_eco_book** out_book
) {
    if (path == nullptr || out_book == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        auto [error, book] = scid::eco::Book::load(path);
        if (error != scid::eco::OK) {
            *out_book = nullptr;
            return error;
        }

        *out_book = new scid_eco_book{std::move(book)};
        return SCID_OK;
    } catch (...) {
        *out_book = nullptr;
        return SCID_ERROR;
    }
}

void scid_eco_book_free(
    scid_eco_book* book
) {
    delete book;
}

scid_error scid_eco_book_code_find(
    const scid_eco_book* book,
    const scid_position* position,
    scid_eco_code* out_code
) {
    if (book == nullptr || position == nullptr || out_code == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        *out_code = book->value.findEco(position->value);
        return SCID_OK;
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_eco_book_name_find(
    const scid_eco_book* book,
    const scid_position* position,
    char* out_text,
    size_t out_text_capacity,
    size_t* out_text_size
) {
    if (book == nullptr || position == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        return write_text(
            eco_name_from_line(book->value.findEcoString(position->value)),
            out_text,
            out_text_capacity,
            out_text_size
        );
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_create_memory(
    const char* name,
    scid_database** out_database
) {
    return database_open(
        "MEMORY",
        scid::database::FMODE_Create,
        name,
        out_database
    );
}

scid_error scid_database_create_scid5(
    const char* path,
    scid_database** out_database
) {
    return database_open(
        "SCID5",
        scid::database::FMODE_Create,
        path,
        out_database
    );
}

scid_error scid_database_open_scid5(
    const char* path,
    scid_database** out_database
) {
    return database_open(
        "SCID5",
        scid::database::FMODE_Both,
        path,
        out_database
    );
}

scid_error scid_database_open_scid5_read_only(
    const char* path,
    scid_database** out_database
) {
    return database_open(
        "SCID5",
        scid::database::FMODE_ReadOnly,
        path,
        out_database
    );
}

scid_error scid_database_close(
    scid_database* database
) {
    if (database == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        if (database->value.isOpen()) {
            database->value.Close();
        }
        database->type.clear();
        return SCID_OK;
    } catch (...) {
        return SCID_ERROR;
    }
}

void scid_database_free(
    scid_database* database
) {
    delete database;
}

scid_error scid_database_is_open(
    const scid_database* database,
    int* out_is_open
) {
    if (database == nullptr || out_is_open == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        return write_bool(database->value.isOpen(), out_is_open);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_filename_get(
    const scid_database* database,
    char* out_text,
    size_t out_text_capacity,
    size_t* out_text_size
) {
    if (database == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        return write_text(
            database->value.getFileName(),
            out_text,
            out_text_capacity,
            out_text_size
        );
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_type_get(
    const scid_database* database,
    char* out_text,
    size_t out_text_capacity,
    size_t* out_text_size
) {
    if (database == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        return write_text(
            database->type,
            out_text,
            out_text_capacity,
            out_text_size
        );
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_read_only_get(
    const scid_database* database,
    int* out_read_only
) {
    if (database == nullptr || out_read_only == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        return write_bool(database->value.isReadOnly(), out_read_only);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_filter_create(
    scid_database* database,
    char* out_filter_id,
    size_t out_filter_id_capacity,
    size_t* out_filter_id_size
) {
    if (database == nullptr || out_filter_id_size == nullptr || !database->value.isOpen()) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        const std::string filter_id = database->value.newFilter();
        const scid_error error = write_text(
            filter_id,
            out_filter_id,
            out_filter_id_capacity,
            out_filter_id_size
        );
        if (error != SCID_OK) {
            database->value.deleteFilter(filter_id.c_str());
            return error;
        }

        return SCID_OK;
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_filter_delete(
    scid_database* database,
    const char* filter_id
) {
    if (database == nullptr ||
        filter_id == nullptr ||
        !database->value.isOpen() ||
        database_filter_id_is_builtin(filter_id)) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        scid::database::HFilter filter(nullptr);
        if (!database_filter_get(database, filter_id, &filter)) {
            return SCID_ERROR_BAD_ARG;
        }

        database->value.deleteFilter(filter_id);
        return SCID_OK;
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_filter_fill(
    scid_database* database,
    const char* filter_id,
    unsigned value
) {
    if (database == nullptr ||
        filter_id == nullptr ||
        !database->value.isOpen() ||
        !database_filter_id_is_mutable(filter_id) ||
        !filter_value_is_valid(value)) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        scid::database::HFilter filter(nullptr);
        if (!database_filter_get(database, filter_id, &filter)) {
            return SCID_ERROR_BAD_ARG;
        }

        filter.mainFilter()->Fill(static_cast<scid::core::byte>(value));
        return SCID_OK;
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_filter_value_set(
    scid_database* database,
    const char* filter_id,
    size_t game_index,
    unsigned value
) {
    if (database == nullptr ||
        filter_id == nullptr ||
        !database->value.isOpen() ||
        !database_filter_id_is_mutable(filter_id) ||
        !filter_value_is_valid(value)) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        scid::database::gamenumT core_game_index = 0;
        if (!database_game_index_is_valid(database->value, game_index, &core_game_index)) {
            return SCID_ERROR_BAD_ARG;
        }

        scid::database::HFilter filter(nullptr);
        if (!database_filter_get(database, filter_id, &filter)) {
            return SCID_ERROR_BAD_ARG;
        }

        filter->set(core_game_index, static_cast<scid::core::byte>(value));
        return SCID_OK;
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_filter_value_get(
    const scid_database* database,
    const char* filter_id,
    size_t game_index,
    unsigned* out_value
) {
    if (database == nullptr ||
        filter_id == nullptr ||
        out_value == nullptr ||
        !database->value.isOpen()) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        scid::database::gamenumT core_game_index = 0;
        if (!database_game_index_is_valid(database->value, game_index, &core_game_index)) {
            return SCID_ERROR_BAD_ARG;
        }

        scid::database::HFilter filter(nullptr);
        if (!database_filter_get(database, filter_id, &filter)) {
            return SCID_ERROR_BAD_ARG;
        }

        *out_value = filter->get(core_game_index);
        return SCID_OK;
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_filter_count_get(
    const scid_database* database,
    const char* filter_id,
    size_t* out_count
) {
    if (database == nullptr ||
        filter_id == nullptr ||
        out_count == nullptr ||
        !database->value.isOpen()) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        scid::database::HFilter filter(nullptr);
        if (!database_filter_get(database, filter_id, &filter)) {
            return SCID_ERROR_BAD_ARG;
        }

        return write_size(filter->size(), out_count);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_filter_game_at_get(
    const scid_database* database,
    const char* filter_id,
    size_t index,
    size_t* out_game_index
) {
    if (database == nullptr ||
        filter_id == nullptr ||
        out_game_index == nullptr ||
        !database->value.isOpen()) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        scid::database::HFilter filter(nullptr);
        if (!database_filter_get(database, filter_id, &filter) ||
            index >= filter->size()) {
            return SCID_ERROR_BAD_ARG;
        }

        size_t ordinal = 0;
        for (const auto core_game_index : *filter) {
            if (ordinal == index) {
                *out_game_index = core_game_index;
                return SCID_OK;
            }
            ++ordinal;
        }

        return SCID_ERROR_BAD_ARG;
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_save(
    scid_database* database
) {
    if (database == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        return database_error_to_c(database->value.flush());
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_metadata_get(
    const scid_database* database,
    const char* key,
    char* out_text,
    size_t out_text_capacity,
    size_t* out_text_size
) {
    if (database == nullptr || key == nullptr || !database->value.isOpen()) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        for (const auto& [name, value] : database->value.getExtraInfo()) {
            if (std::strcmp(name, key) == 0) {
                return write_text(value, out_text, out_text_capacity, out_text_size);
            }
        }

        return write_text("", out_text, out_text_capacity, out_text_size);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_metadata_set(
    scid_database* database,
    const char* key,
    const char* value
) {
    if (database == nullptr ||
        key == nullptr ||
        value == nullptr ||
        !database->value.isOpen()) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        return database_error_to_c(database->value.setExtraInfo(key, value));
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_metadata_count_get(
    const scid_database* database,
    size_t* out_count
) {
    if (database == nullptr || out_count == nullptr || !database->value.isOpen()) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        return write_size(database->value.getExtraInfo().size(), out_count);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_metadata_at_get(
    const scid_database* database,
    size_t index,
    char* out_key,
    size_t out_key_capacity,
    size_t* out_key_size,
    char* out_value,
    size_t out_value_capacity,
    size_t* out_value_size
) {
    if (database == nullptr || !database->value.isOpen()) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        const auto metadata = database->value.getExtraInfo();
        if (index >= metadata.size()) {
            return SCID_ERROR_BAD_ARG;
        }

        const auto& [key, value] = metadata[index];
        if (const scid_error error =
                write_text(key, out_key, out_key_capacity, out_key_size);
            error != SCID_OK) {
            return error;
        }

        return write_text(value, out_value, out_value_capacity, out_value_size);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_stats_date_range_get(
    const scid_database* database,
    char* out_min_date,
    size_t out_min_date_capacity,
    size_t* out_min_date_size,
    char* out_max_date,
    size_t out_max_date_capacity,
    size_t* out_max_date_size
) {
    if (database == nullptr ||
        out_min_date_size == nullptr ||
        out_max_date_size == nullptr ||
        !database->value.isOpen()) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        const auto& stats = database->value.getStats();
        if (const scid_error error = write_text(
                date_to_string(stats.minDate),
                out_min_date,
                out_min_date_capacity,
                out_min_date_size
            );
            error != SCID_OK) {
            return error;
        }

        return write_text(
            date_to_string(stats.maxDate),
            out_max_date,
            out_max_date_capacity,
            out_max_date_size
        );
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_stats_result_count_get(
    const scid_database* database,
    const char* result,
    size_t* out_count
) {
    if (database == nullptr ||
        result == nullptr ||
        out_count == nullptr ||
        !database->value.isOpen()) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        scid::core::resultT core_result = scid::core::RESULT_None;
        if (const scid_error error = result_from_string(result, &core_result);
            error != SCID_OK) {
            return error;
        }

        return write_size(database->value.getStats().nResults[core_result], out_count);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_game_count_get(
    const scid_database* database,
    size_t* out_count
) {
    if (database == nullptr || out_count == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        return write_size(database->value.numGames(), out_count);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_import_pgn(
    scid_database* database,
    const char* pgn,
    size_t pgn_size,
    char* out_diagnostic,
    size_t out_diagnostic_capacity,
    size_t* out_diagnostic_size,
    size_t* out_imported_count
) {
    if (database == nullptr || pgn == nullptr || out_imported_count == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    *out_imported_count = 0;

    try {
        std::vector<scid::core::Game> games;
        scid::core::pgn::ParseLog log;
        size_t parsed_size = 0;

        while (parsed_size < pgn_size) {
            while (parsed_size < pgn_size &&
                   std::isspace(static_cast<unsigned char>(pgn[parsed_size]))) {
                ++parsed_size;
            }
            if (parsed_size >= pgn_size) {
                break;
            }

            scid::core::Game game;
            const auto start_size = log.n_bytes;
            const bool ok = scid::core::pgn::parseGame(
                pgn + parsed_size,
                pgn_size - parsed_size,
                game,
                log
            );
            const auto consumed_size =
                static_cast<size_t>(log.n_bytes - start_size);

            if (!ok || consumed_size == 0) {
                const scid_error diagnostic_error = write_optional_diagnostic(
                    log.log,
                    out_diagnostic,
                    out_diagnostic_capacity,
                    out_diagnostic_size
                );
                return diagnostic_error == SCID_OK ? SCID_ERROR_CORRUPT
                                                   : diagnostic_error;
            }

            parsed_size += consumed_size;
            games.push_back(std::move(game));
        }

        const scid_error diagnostic_error = write_optional_diagnostic(
            log.log,
            out_diagnostic,
            out_diagnostic_capacity,
            out_diagnostic_size
        );
        if (diagnostic_error != SCID_OK) {
            return diagnostic_error;
        }

        for (const auto& game : games) {
            const scid_error error =
                database_error_to_c(database->value.addGame(game, ""));
            if (error != SCID_OK) {
                return error;
            }
            ++*out_imported_count;
        }

        return SCID_OK;
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_game_add(
    scid_database* database,
    const scid_game* game,
    const char* flags
) {
    if (database == nullptr || game == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        const auto error = database->value.addGame(
            game->value,
            flags == nullptr ? "" : flags
        );
        return database_error_to_c(error);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_game_replace(
    scid_database* database,
    size_t index,
    const scid_game* game,
    const char* flags
) {
    if (database == nullptr || game == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        scid::database::gamenumT game_index = 0;
        if (!database_game_index_is_valid(database->value, index, &game_index)) {
            return SCID_ERROR_BAD_ARG;
        }

        return database_error_to_c(database->value.saveGame(
            game->value,
            flags == nullptr ? "" : flags,
            game_index
        ));
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_game_delete(
    scid_database* database,
    size_t index
) {
    if (database == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        scid::database::gamenumT game_index = 0;
        if (!database_game_index_is_valid(database->value, index, &game_index)) {
            return SCID_ERROR_BAD_ARG;
        }

        return database_error_to_c(database->value.setFlag(
            true,
            1u << scid::database::GAME_FLAG_DELETE,
            game_index
        ));
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_game_undelete(
    scid_database* database,
    size_t index
) {
    if (database == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        scid::database::gamenumT game_index = 0;
        if (!database_game_index_is_valid(database->value, index, &game_index)) {
            return SCID_ERROR_BAD_ARG;
        }

        return database_error_to_c(database->value.setFlag(
            false,
            1u << scid::database::GAME_FLAG_DELETE,
            game_index
        ));
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_game_get(
    const scid_database* database,
    size_t index,
    scid_game** out_game,
    char* out_flags,
    size_t out_flags_capacity,
    size_t* out_flags_size
) {
    if (database == nullptr || out_game == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        auto* game = new scid_game;
        std::array<char, 22> flags = {};
        const auto error = database->value.loadGame(
            static_cast<scid::database::gamenumT>(index),
            game->value,
            flags.data(),
            flags.size()
        );
        if (error != scid::core::OK) {
            delete game;
            *out_game = nullptr;
            return error == scid::core::ERROR_BadArg
                       ? SCID_ERROR_BAD_ARG
                       : SCID_ERROR;
        }

        if (const scid_error flags_error = write_optional_text(
                flags.data(),
                out_flags,
                out_flags_capacity,
                out_flags_size);
            flags_error != SCID_OK) {
            delete game;
            *out_game = nullptr;
            return flags_error;
        }

        *out_game = game;
        return SCID_OK;
    } catch (...) {
        *out_game = nullptr;
        return SCID_ERROR;
    }
}

scid_error scid_database_game_export_pgn(
    const scid_database* database,
    size_t index,
    char* out_text,
    size_t out_text_capacity,
    size_t* out_text_size
) {
    if (database == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        scid::database::gamenumT game_index = 0;
        if (!database_game_index_to_core(index, &game_index)) {
            return SCID_ERROR_BAD_ARG;
        }

        scid::core::Game game;
        const auto error = database->value.loadGame(
            game_index,
            game,
            nullptr,
            0
        );
        if (error != scid::core::OK) {
            return database_error_to_c(error);
        }

        std::string pgn;
        scid::core::pgn::encode(game, pgn);
        return write_text(pgn, out_text, out_text_capacity, out_text_size);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_game_tag_get(
    const scid_database* database,
    size_t index,
    const char* name,
    char* out_text,
    size_t out_text_capacity,
    size_t* out_text_size
) {
    if (database == nullptr || name == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        scid::database::gamenumT game_index = 0;
        if (!database_game_index_to_core(index, &game_index) ||
            !database->value.gameInfoBounds(game_index)) {
            return SCID_ERROR_BAD_ARG;
        }

        return write_text(
            database_game_tag_value(database->value, game_index, name),
            out_text,
            out_text_capacity,
            out_text_size
        );
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_game_halfmove_count_get(
    const scid_database* database,
    size_t index,
    size_t* out_count
) {
    if (database == nullptr || out_count == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        scid::database::gamenumT game_index = 0;
        if (!database_game_index_to_core(index, &game_index)) {
            return SCID_ERROR_BAD_ARG;
        }

        const auto info = database->value.gameInfoBounds(game_index);
        if (!info) {
            return SCID_ERROR_BAD_ARG;
        }

        return write_size(info->halfMoveCount, out_count);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_game_number_get(
    const scid_database* database,
    size_t index,
    size_t* out_number
) {
    if (database == nullptr || out_number == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        scid::database::GameInfo info;
        if (!database_game_info_get(database->value, index, &info)) {
            return SCID_ERROR_BAD_ARG;
        }

        return write_size(index + 1, out_number);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_game_deleted_get(
    const scid_database* database,
    size_t index,
    int* out_deleted
) {
    if (database == nullptr || out_deleted == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        scid::database::GameInfo info;
        if (!database_game_info_get(database->value, index, &info)) {
            return SCID_ERROR_BAD_ARG;
        }

        return write_bool(info.hasDeleteFlag(), out_deleted);
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_game_result_get(
    const scid_database* database,
    size_t index,
    char* out_text,
    size_t out_text_capacity,
    size_t* out_text_size
) {
    if (database == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        scid::database::GameInfo info;
        if (!database_game_info_get(database->value, index, &info)) {
            return SCID_ERROR_BAD_ARG;
        }

        return write_text(
            scid::core::RESULT_LONGSTR[info.result],
            out_text,
            out_text_capacity,
            out_text_size
        );
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_game_eco_get(
    const scid_database* database,
    size_t index,
    scid_eco_code* out_code
) {
    if (database == nullptr || out_code == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        scid::database::GameInfo info;
        if (!database_game_info_get(database->value, index, &info)) {
            return SCID_ERROR_BAD_ARG;
        }

        *out_code = static_cast<scid_eco_code>(info.ecoCode);
        return SCID_OK;
    } catch (...) {
        return SCID_ERROR;
    }
}

scid_error scid_database_game_date_get(
    const scid_database* database,
    size_t index,
    char* out_text,
    size_t out_text_capacity,
    size_t* out_text_size
) {
    if (database == nullptr) {
        return SCID_ERROR_BAD_ARG;
    }

    try {
        scid::database::GameInfo info;
        if (!database_game_info_get(database->value, index, &info)) {
            return SCID_ERROR_BAD_ARG;
        }

        return write_text(
            date_to_string(info.date),
            out_text,
            out_text_capacity,
            out_text_size
        );
    } catch (...) {
        return SCID_ERROR;
    }
}
