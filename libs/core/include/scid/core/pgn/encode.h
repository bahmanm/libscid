/*
 * Copyright (C) 2022  Fulvio Benini.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH
 * THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/** @file
 * PGN encoder entry points and formatting helpers.
 *
 * The encoder writes core Game objects as PGN.  Low-level helpers first use
 * NUL bytes as token separators, then break_lines() converts those separators
 * into spaces or newlines for final text output.
 */

#pragma once

#include "scid/core/game.h"
#include "scid/core/nags.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace scid::core::pgn {

/** Options controlling which PGN content is emitted. */
struct EncodeOptions {
	/** Emit symbolic NAGs such as @c ! when available instead of numeric @c $1. */
	bool symbolicNags = false;
	/** Emit non-seven-tag supplemental tags such as ratings, ECO, EventDate, and extras. */
	bool includeSupplementalTags = true;
	/** Emit initial, move, and variation comments, plus NAGs. */
	bool includeComments = true;
	/** Emit child variations recursively. */
	bool includeVariations = true;
	/** Runtime movetext line width.  When empty, the template width is used. */
	std::optional<unsigned> lineWidth = std::nullopt;
};

/** Replaces internal token separators with spaces or newlines.
 *
 * The encoder uses NUL bytes as soft token breakpoints while constructing PGN.
 * This helper turns those breakpoints into spaces, promoting the most recent
 * breakpoint to a newline when the current line would exceed @p desired_len.
 * Existing newlines are preserved, so comments and tag values are not split
 * unless @p hard_len requests a secondary pass that may break on spaces.
 *
 * @returns an iterator to the first character of the final line.
 */
template <int desired_len = 80, char breakpoint_char = '\0', int hard_len = 0,
          typename Iter>
Iter break_lines(Iter begin, Iter end) {
	auto line_first_char = begin;
	auto last_breakpoint = begin;
	auto it = begin;
	while (true) {
		it = find_if(it, end, [&](char ch) {
			return ch == '\n' || ch == breakpoint_char;
		});

		// Change the last breakpoint to newline char if the line would exceed
		// the desired length and there weren't newline chars (for example in
		// comments) beetween this and the last breakpoint.
		if (std::distance(line_first_char, it) > desired_len &&
		    last_breakpoint > line_first_char) {
			*last_breakpoint = '\n';
			line_first_char = last_breakpoint + 1;
		}

		// If a secondary line length was requested, try to convert spaces to
		// newline chars (this is not desiderable, but old software may use
		// limited fixed size buffer when reading PGNs).
		if (hard_len != 0 && std::distance(line_first_char, it) > hard_len) {
			line_first_char = break_lines<hard_len, ' '>(line_first_char, it);
		}

		if (it == end)
			break;

		if (*it == '\n') {
			line_first_char = ++it;
		} else /*  *it == breakpoint_char  */ {
			last_breakpoint = it;
			*it++ = ' ';
		}
	}
	return line_first_char;
}

/** Runtime-width overload of break_lines(). */
template <typename Iter>
Iter break_lines(Iter begin, Iter end, unsigned desired_len) {
	auto line_first_char = begin;
	auto last_breakpoint = begin;
	auto it = begin;
	while (true) {
		it = find_if(it, end, [](char ch) {
			return ch == '\n' || ch == '\0';
		});

		if (desired_len != 0 &&
		    std::distance(line_first_char, it) > desired_len &&
		    last_breakpoint > line_first_char) {
			*last_breakpoint = '\n';
			line_first_char = last_breakpoint + 1;
		}

		if (it == end)
			break;

		if (*it == '\n') {
			line_first_char = ++it;
		} else {
			last_breakpoint = it;
			*it++ = ' ';
		}
	}
	return line_first_char;
}

/** Escapes quotes and backslashes in a PGN string token.
 *
 * Only the substring beginning at @p pos is processed, which lets callers append
 * a tag value and then escape just the newly appended content.
 */
template <typename TCont>
void escape_string(TCont& str, typename TCont::size_type pos) {
	auto it = str.begin() + pos;
	while (true) {
		it = std::find_if(it, str.end(),
		                  [](char ch) { return ch == '\\' || ch == '\"'; });
		if (it != str.end())
			it = str.insert(it, '\\') + 2;
		else
			break;
	}
}

/** Appends one PGN tag pair to @p dest.
 *
 * The tag/value separator is written as NUL so that break_lines() can later
 * convert it to a space.  Tag values are escaped according to PGN string-token
 * rules.
 *
 * @tparam unknown_to_question_mark when true, empty Event, Site, Round, White,
 * and Black values are written as @c ?.
 */
template <bool unknown_to_question_mark = false, typename TCont>
void encode_tag_pair(std::string_view tag, std::string_view value,
                     TCont& dest) {
	dest.push_back('[');
	dest.insert(dest.end(), tag.begin(), tag.end());
	dest.push_back('\0');

	dest.push_back('"');
	if (unknown_to_question_mark && value.empty() &&
	    (tag == "Event" || tag == "Site" || tag == "Round" || tag == "White" ||
	     tag == "Black")) {
		dest.push_back('?');
	} else {
		auto value_begin = dest.size();
		dest.insert(dest.end(), value.begin(), value.end());
		escape_string(dest, value_begin);
	}
	dest.push_back('"');

	dest.push_back(']');
	// "Each tag pair should appear left justified on a line by itself"
	dest.push_back('\n');
}

/** Attempts to encode @p comment as a PGN semicolon comment.
 *
 * Semicolon comments run to the end of the line.  Comments containing line
 * breaks, NUL separators, or exceeding @p hard_len are rejected without
 * modifying @p dest.
 *
 * @returns true when @p comment was appended.
 */
template <int hard_len = 0, typename TCont>
[[nodiscard]] bool encode_comment_rest_of_line(std::string_view comment,
                                               TCont& dest) {
	if ((hard_len != 0 && comment.size() >= hard_len) ||
	    std::any_of(comment.begin(), comment.end(),
	                [](char ch) { return ch == '\n' || ch == '\0'; }))
		return false;

	if (!dest.empty() && dest.back() != '\0' && dest.back() != '\n') {
		dest.push_back('\0');
	}
	dest.push_back(';');
	dest.insert(dest.end(), comment.begin(), comment.end());
	dest.push_back('\n');
	return true;
}

/** Appends @p comment using a legal PGN comment form.
 *
 * Brace comments are preferred.  If the comment contains braces, the helper
 * first tries semicolon-comment form; when that is not possible, braces inside
 * the comment are replaced with UTF-8 fullwidth brace characters.
 */
template <int hard_len = 0, typename TCont>
static void encode_comment(std::string_view comment, TCont& dest) {
	auto is_curly = [](char ch) { return ch == '{' || ch == '}'; };
	auto it_curly = std::find_if(comment.begin(), comment.end(), is_curly);
	if (it_curly != comment.end() &&
	    encode_comment_rest_of_line<hard_len>(comment, dest))
		return;

	dest.push_back('{');
	dest.insert(dest.end(), comment.begin(), comment.end());
	if (it_curly != comment.end()) {
		// Replace curly braces with UTF-8 fullwidth curly braces U+FF5B
		// (ef bd 9b) or U+FF5D (ef bd 9d).
		auto it = dest.end() - std::distance(it_curly, comment.end());
		do {
			auto replace_char = (*it == '{') ? u8"\uFF5B" : u8"\uFF5D";
			static_assert(std::u8string_view(u8"\uFF5D").size() == 3);
			it = dest.insert(it, 2, '\0');
			it = std::copy_n(replace_char, 3, it);

			it = std::find_if(it, dest.end(), is_curly);
		} while (it != dest.end());
	}
	dest.push_back('}');
	dest.push_back('\0');
}

namespace detail {

/** Internal entry kinds used while linearising movetext. */
enum class MovetextEntryKind {
	InitialComment,
	VariationStart,
	VariationEnd,
	Move
};

/** Internal flattened movetext entry passed to encode_movetext_entry(). */
struct MovetextEntry {
	MovetextEntryKind kind;
	std::string_view san;
	std::string_view comment;
	std::span<const Nag> nags;
};

/** Returns a move's stored SAN or computes SAN from @p position. */
inline std::string san_for_move(scid::core::Position& position,
                                const Move& move,
                                scid::core::sanFlagT flag) {
	if (!move.san.empty())
		return move.san;

	return position.makeSan(move.spec, flag);
}

/** Appends a flattened movetext entry to @p dest. */
template <int hard_len = 0, typename TCont>
void encode_movetext_entry(MovetextEntry const& entry,
                           std::vector<long long>& ply,
                           typename TCont::size_type& move_end,
                           TCont& dest,
                           EncodeOptions options) {
	switch (entry.kind) {
	case MovetextEntryKind::InitialComment:
		if (options.includeComments && !entry.comment.empty())
			encode_comment<hard_len>(entry.comment, dest);
		break;

	case MovetextEntryKind::VariationStart:
		ply.push_back(ply.back() - 1);
		dest.push_back('(');
		if (options.includeComments && !entry.comment.empty())
			encode_comment<hard_len>(entry.comment, dest);
		break;

	case MovetextEntryKind::VariationEnd:
		ply.pop_back();
		if (dest.back() == '\0') {
			dest.back() = ')';
		} else {
			dest.push_back(')');
		}
		dest.push_back('\0');
		break;

	case MovetextEntryKind::Move: {
		auto white_to_move = (ply.back() % 2) == 0;
		if (white_to_move || move_end != dest.size()) {
			auto move_number = std::to_string(ply.back() / 2 + 1);
			move_number.append(white_to_move ? 1 : 3, '.');
			dest.insert(dest.end(), move_number.begin(), move_number.end());
		}
		dest.insert(dest.end(), entry.san.begin(), entry.san.end());
		dest.push_back('\0');
		move_end = dest.size();
		ply.back()++;

		if (options.includeComments) {
			for (auto nag : entry.nags) {
				auto nag_str = nagToString(nag, options.symbolicNags);
				dest.insert(dest.end(), nag_str.begin(), nag_str.end());
				dest.push_back('\0');
			}
		}
		if (options.includeComments && !entry.comment.empty())
			encode_comment<hard_len>(entry.comment, dest);
		break;
	}
	}
}

} // namespace detail

/** Encodes one move sequence and its child variations. */
template <int hard_len = 0, typename TCont>
void encode_core_line(MoveSequence const& line,
                      scid::core::Position position,
                      std::vector<long long>& ply,
                      typename TCont::size_type& move_end, TCont& dest,
                      EncodeOptions options = {}) {
	for (std::size_t i = 0; i < line.moves.size(); ++i) {
		auto const& move = line.moves[i];
		auto position_before_move = position;
		const auto sanFlag = i + 1 == line.moves.size()
		                         ? scid::core::SAN_MATETEST
		                         : scid::core::SAN_CHECKTEST;
		const auto san = detail::san_for_move(position, move, sanFlag);

		detail::encode_movetext_entry<hard_len>(
		    {detail::MovetextEntryKind::Move,
		     san,
		     move.metadata.comment,
		     {move.metadata.nags.data(), move.metadata.nags.size()}},
		    ply, move_end, dest, options);

		if (options.includeVariations) {
			for (auto const& variation : move.childVariations) {
				detail::encode_movetext_entry<hard_len>(
				    {detail::MovetextEntryKind::VariationStart,
				     {},
				     variation.initialComment,
				     {}},
				    ply, move_end, dest, options);
				encode_core_line<hard_len>(variation.line, position_before_move,
				                           ply, move_end, dest, options);
				detail::encode_movetext_entry<hard_len>(
				    {detail::MovetextEntryKind::VariationEnd, {}, {}, {}},
				    ply, move_end, dest, options);
			}
		}

		(void)position.applyMove(move.spec);
	}
}

/** Encodes only a game's movetext section.
 *
 * The destination receives a leading blank line before movetext, matching the
 * separation between PGN tag pairs and movetext in a complete game.
 */
template <int hard_len = 0, typename TCont>
void encode_movetext(Game const& game, TCont& dest,
                     EncodeOptions options = {}) {
	std::vector<long long> ply = {game.initialPlyCounter()};
	auto move_end = dest.size();
	dest.push_back('\n');

	if (options.includeComments && !game.initialComment().empty()) {
		detail::encode_movetext_entry<hard_len>(
		    {detail::MovetextEntryKind::InitialComment,
		     {},
		     game.initialComment(),
		     {}},
		    ply, move_end, dest, options);
	}

	auto position = game.startPosition() ? *game.startPosition()
	                                     : scid::core::Position::getStdStart();
	encode_core_line<hard_len>(game.movetext().mainline, position, ply,
	                           move_end, dest, options);

	if (dest.back() == '\0')
		dest.back() = '\n';
}

/** Encodes the seven tag roster plus selected supplemental tags. */
template <typename TCont>
void encode_core_tag_pairs(Game const& game, TCont& dest,
                           EncodeOptions options = {}) {
	char str_buf[256];
	encode_tag_pair("Event", game.event(), dest);
	encode_tag_pair("Site", game.site(), dest);
	scid::core::date_DecodeToString(game.date(), str_buf);
	encode_tag_pair("Date", str_buf, dest);
	encode_tag_pair("Round", game.round(), dest);
	encode_tag_pair("White", game.white().name, dest);
	encode_tag_pair("Black", game.black().name, dest);
	encode_tag_pair("Result", game.resultString(), dest);

	if (options.includeSupplementalTags) {
		if (auto rating = game.white().rating.value) {
			std::string tag = "White";
			tag.append(scid::core::ratingTypeNames[game.white().rating.type]);
			encode_tag_pair(tag, std::to_string(rating), dest);
		}
		if (auto rating = game.black().rating.value) {
			std::string tag = "Black";
			tag.append(scid::core::ratingTypeNames[game.black().rating.type]);
			encode_tag_pair(tag, std::to_string(rating), dest);
		}
		if (!game.eco().empty())
			encode_tag_pair("ECO", game.eco(), dest);
		if (game.eventDate() != scid::core::ZERO_DATE) {
			scid::core::date_DecodeToString(game.eventDate(), str_buf);
			encode_tag_pair("EventDate", str_buf, dest);
		}
		for (auto const& tag : game.extraTags())
			encode_tag_pair(tag.first, tag.second, dest);
	}
	if (game.hasNonStandardStart(str_buf, sizeof(str_buf)))
		encode_tag_pair("FEN", str_buf, dest);
}

/** Encodes a complete game without final line wrapping.
 *
 * Token separators remain as NUL bytes.  Use encode() for normal PGN text.
 */
template <int hard_len = 0, typename TCont>
void encode_game(Game const& game, TCont& dest, EncodeOptions options = {}) {
	encode_core_tag_pairs(game, dest, options);
	encode_movetext<hard_len>(game, dest, options);

	auto result = game.resultString();
	dest.insert(dest.end(), result.begin(), result.end());
	dest.push_back('\n');
}

/** Encodes a complete game as final PGN text.
 *
 * The generated game is appended to @p dest.  Internal NUL token separators are
 * converted to spaces or line breaks before the function returns.
 */
template <int desired_len = 80, typename TGame, typename TCont>
void encode(TGame const& game, TCont& dest, EncodeOptions options = {}) {
	auto begin = dest.size();
	encode_game(game, dest, options);
	if (options.lineWidth) {
		break_lines(dest.begin() + begin, dest.end(), *options.lineWidth);
	} else {
		break_lines<desired_len>(dest.begin() + begin, dest.end());
	}
}

} // namespace scid::core::pgn
