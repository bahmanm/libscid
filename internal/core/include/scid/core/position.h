/** @file
 * Chess position state, move generation, and notation support.
 */

#pragma once

#include "scid/core/error.h"
#include "scid/core/move.h"
#include "scid/core/movelist.h"
#include "scid/core/notation.h"

#include <cassert>
#include <cstddef>
#include <stdio.h>
#include <string>
#include <string_view>

namespace scid::core
{

    class DString;
    class SquareSet;
    class SquareList;

    //////////////////////////////////////////////////////////////////////
    //  Position:  Constants

    /** Castling-right bit for White queenside castling. */
    const byte WQ_CASTLE = 1;
    /** Castling-right bit for White kingside castling. */
    const byte WK_CASTLE = 2;
    /** Castling-right bit for Black queenside castling. */
    const byte BQ_CASTLE = 4;
    /** Castling-right bit for Black kingside castling. */
    const byte BK_CASTLE = 8;

    /** Move-generation filter flags. */
    typedef uint genMovesT;
    /** Generate capturing moves only. */
    const genMovesT GEN_CAPTURES = 1;
    /** Generate non-capturing moves only. */
    const genMovesT GEN_NON_CAPS = 2;
    /** Generate all legal moves. */
    const genMovesT GEN_ALL_MOVES = (GEN_CAPTURES | GEN_NON_CAPS);

    ///////////////////////////////////////////////////////////////////////////
    //  Position:  Class definition

    /** Complete chess position with move generation, validation, and notation.
     *
     * Position owns board occupancy, side to move, castling rights, en-passant
     * state, move counters, piece lists, material counters, and hash keys.  It is
     * the authority for resolving portable MoveSpec requests into reversible
     * MoveAction records and for applying or undoing those actions.
     *
     * The default constructor prepares internal sentinels but does not set a legal
     * chess position.  Call StdStart(), Clear(), ReadFromFEN(), or AddPiece() setup
     * routines before using move generation or legality checks.
     *
     * Parsing and bulk-application methods mutate this object as they work.  When
     * callers need all-or-nothing replacement semantics, parse into a temporary
     * Position first and assign it only after the operation returns OK.
     */
    class Position
    {

        private:
            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            //  Position:  Data structures

            pieceT Board[66];       // the actual board + a color square
                                    // and a NULL square.
            uint Count[2];          // count of pieces & pawns each
            byte Material[16] = {}; // count of each type of piece
            byte ListPos[64] = {};  // ListPos stores the position in
                                    // List[][] for the piece on
                                    // square x.
            squareT List[2][16];    // list of piece squares for each side
            byte    NumOnRank[16][8] = {};
            byte    NumOnFyle[16][8] = {};
            byte    NumOnLeftDiag[16][16] = {}; // Num Queens/Bishops
            byte    NumOnRightDiag[16][16] = {};
            byte    NumOnSquareColor[16][2] = {};

            directionT Pinned[16]; // For each List[ToMove][x], stores
                                   // whether piece is pinned to its
                                   // own king and dir from king.

            squareT EPTarget; // square pawns can EP capture to
            colorT  ToMove;
            ushort  HalfMoveClock; // Count of halfmoves since last capture
                                   // or pawn move.
            ushort  PlyCounter;
            byte    Castling = 0;     // castling flags
            byte    variant_ = 0;     // 0 -> normal; 1 -> chess960
            squareT castleRookSq_[4]; // start rook squares

            uint Hash;     // Hash value.
            uint PawnHash; // Pawn structure hash value.

            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            //  Position:  Private Functions

            inline void
            AddHash(
                pieceT  p,
                squareT sq);
            inline void
            UnHash(
                pieceT  p,
                squareT sq);

            inline void
            AddToBoard(
                pieceT  p,
                squareT sq);
            inline void
            RemoveFromBoard(
                pieceT  p,
                squareT sq);

            void
            CalcPinsDir(
                directionT dir,
                pieceT     attacker);

            void
            GenSliderMoves(
                MoveList*  mlist,
                colorT     c,
                squareT    sq,
                directionT dir,
                SquareSet* sqset,
                bool       capturesOnly);
            void
            GenKnightMoves(
                MoveList*  mlist,
                colorT     c,
                squareT    sq,
                SquareSet* sqset,
                bool       capturesOnly);

            void
            AddLegalMove(
                MoveList* mlist,
                squareT   from,
                squareT   to,
                pieceT    promo);
            void
            GenCastling(MoveList* mlist);
            void
            GenKingMoves(
                MoveList* mlist,
                genMovesT genType);
            void
            AddPromotions(
                MoveList* mlist,
                squareT   from,
                squareT   dest);
            bool
            IsValidEnPassant(
                squareT from,
                squareT to);
            void
            GenPawnMoves(
                MoveList*  mlist,
                squareT    from,
                directionT dir,
                SquareSet* sqset,
                genMovesT  genType);

            void
            GenCheckEvasions(
                MoveList*   mlist,
                pieceT      mask,
                genMovesT   genType,
                SquareList* checkSquares);

            errorT
            readPieceMoveAction(
                MoveAction* sm,
                const char* str,
                size_t      slen,
                pieceT      p) const;
            errorT
            readCastleMoveAction(
                MoveAction*      sm,
                std::string_view str) const;
            errorT
            readPawnMoveAction(
                MoveAction* sm,
                const char* str,
                size_t      slen,
                fyleT       from);
            errorT
            readKingMoveAction(
                MoveAction* sm,
                const char* str,
                size_t      slen) const;
            errorT
            readCoordinateMoveAction(
                MoveAction* m,
                const char* s,
                size_t      slen,
                bool        reverse);
            errorT
            parseMoveAction(
                MoveAction* sm,
                const char* begin,
                const char* end);
            void
            fillMoveAction(MoveAction& sm) const;
            void
            resolveMove(
                squareT     from,
                squareT     to,
                pieceT      promo,
                MoveAction& action) const;

            template <typename TFunc>
            bool
            under_attack(
                squareT target_sq,
                squareT captured_sq,
                TFunc   not_empty) const;
            bool
            under_attack(squareT target_sq) const;

            static constexpr unsigned
            castlingIdx(
                colorT     color,
                castleDirT side)
            {
                return 2 * color + side;
            }
            squareT
            find_castle_rook(
                colorT  col,
                squareT rsq) const;
            squareT
            castleRookSq(
                colorT color,
                bool   king_side) const
            {
                return castleRookSq_[2 * color + (king_side ? 1 : 0)];
            }

            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            //  Position:  Public Functions
        public:
            /** Constructs an empty position object with internal sentinel squares set. */
            Position();
            /** Returns a shared standard starting position. */
            static const Position&
            getStdStart();

            /** Clears the board, counters, castling rights, en-passant target, and hash. */
            void
            Clear();
            /** Replaces this position with the standard starting position. */
            void
            StdStart()
            {
                *this = getStdStart();
            }
            /** Returns true when this position exactly matches the standard start. */
            bool
            IsStdStart() const;
            /** Adds @p p on @p sq while maintaining piece lists, material, and hashes. */
            errorT
            AddPiece(
                pieceT  p,
                squareT sq);

            /** Returns true when the position uses Chess960 castling rook squares. */
            bool
            isChess960() const
            {
                return variant_ == 1;
            }

            // Set and Get attributes -- one-liners
            /** Returns the number of pieces of the exact piece code @p p. */
            byte
            PieceCount(pieceT p)
            {
                return Material[p];
            }
            /** Returns the raw material counter array indexed by piece code. */
            const byte*
            GetMaterial() const
            {
                return Material;
            }
            /** Sets the en-passant target square, or NULL_SQUARE when none exists. */
            void
            SetEPTarget(squareT s)
            {
                EPTarget = s;
            }
            /** Returns the en-passant target square, or NULL_SQUARE. */
            squareT
            GetEPTarget() const
            {
                return EPTarget;
            }
            /** Sets the side to move. */
            void
            SetToMove(colorT c)
            {
                ToMove = c;
            }
            /** Returns the side to move. */
            colorT
            GetToMove() const
            {
                return ToMove;
            }
            /** Returns true when White is to move. */
            bool
            WhiteToMove() const
            {
                return ToMove == WHITE;
            }
            /** Sets the halfmove ply counter used for full-move numbering. */
            void
            SetPlyCounter(ushort x)
            {
                PlyCounter = x;
            }
            /** Returns the halfmove ply counter. */
            ushort
            GetPlyCounter() const
            {
                return PlyCounter;
            }
            /** Returns the halfmove clock used by the 50-move rule. */
            ushort
            GetHalfMoveClock() const
            {
                return HalfMoveClock;
            }
            /** Returns the one-based full-move number derived from the ply counter. */
            ushort
            GetFullMoveCount() const
            {
                return PlyCounter / 2 + 1;
            }

            // Methods to get the Board or piece lists -- used in game.cpp to
            // decode moves:
            /** Returns the raw piece-square list for @p c. */
            const squareT*
            GetList(colorT c) const
            {
                return List[c];
            }
            /** Returns the number of pieces currently held by @p c. */
            uint
            GetCount(colorT c) const
            {
                return Count[c];
            }
            /** Returns the total number of pieces on the board. */
            uint
            TotalMaterial() const
            {
                return Count[WHITE] + Count[BLACK];
            }
            /** Returns the number of non-pawn pieces for @p c, including the king. */
            uint
            NumNonPawns(colorT c)
            {
                return Count[c] - Material[piece_Make(c, PAWN)];
            }
            /** Returns true when both sides have only king and pawns. */
            bool
            InPawnEnding()
            {
                return (NumNonPawns(WHITE) == 1 && NumNonPawns(BLACK) == 1);
            }
            /** Returns a simple material value for @p c using Q=9, R=5, B/N=3, P=1. */
            uint
            MaterialValue(colorT c);
            /** Returns the count of exact piece @p p on file @p f. */
            inline uint
            FyleCount(
                pieceT p,
                fyleT  f) const
            {
                return NumOnFyle[p][f];
            }
            /** Returns the count of exact piece @p p on rank @p r. */
            inline uint
            RankCount(
                pieceT p,
                rankT  r) const
            {
                return NumOnRank[p][r];
            }
            /** Returns the count of exact piece @p p on left diagonal @p diag. */
            inline uint
            LeftDiagCount(
                pieceT    p,
                leftDiagT diag) const
            {
                return NumOnLeftDiag[p][diag];
            }
            /** Returns the count of exact piece @p p on right diagonal @p diag. */
            inline uint
            RightDiagCount(
                pieceT     p,
                rightDiagT diag) const
            {
                return NumOnRightDiag[p][diag];
            }
            /** Returns the count of exact piece @p p on dark or light squares. */
            inline uint
            SquareColorCount(
                pieceT p,
                colorT sqColor) const
            {
                return NumOnSquareColor[p][sqColor];
            }

            /** Returns the raw board array with side-to-move encoded in COLOR_SQUARE. */
            const pieceT*
            GetBoard() const
            {
                const_cast<Position*>(this)->Board[COLOR_SQUARE] = COLOR_CHAR[ToMove];
                return Board;
            }

            /** Returns the piece on @p sq, or EMPTY. */
            pieceT
            GetPiece(squareT sq) const
            {
                assert(sq < 64);
                return Board[sq];
            }

            // Other one-line methods
            /** Returns the king square for @p c. */
            squareT
            GetKingSquare(colorT c) const
            {
                return List[c][0];
            }
            /** Returns the king square for the side to move. */
            squareT
            GetKingSquare() const
            {
                return List[ToMove][0];
            }
            /** Returns the king square for the side not to move. */
            squareT
            GetEnemyKingSquare() const
            {
                return List[1 - ToMove][0];
            }

            // Castling flags
            /** Returns whether @p c currently has the castling right @p dir. */
            bool
            GetCastling(
                colorT     c,
                castleDirT dir) const
            {
                return Castling & (1u << castlingIdx(c, dir));
            }
            /** Returns all castling-right bits. */
            byte
            GetCastlingFlags() const
            {
                return Castling;
            }
            /** Validates that the stored castling right still matches king and rook placement. */
            bool
            validCastlingFlag(
                colorT color,
                bool   king_side) const;

            // Hashing
            /** Returns the full position hash. */
            inline uint
            HashValue(void) const
            {
                return Hash;
            }
            /** Returns the pawn-structure hash. */
            inline uint
            PawnHashValue(void) const
            {
                return PawnHash;
            }
            /** Returns the historic position signature used by database code. */
            uint
            GetHPSig();

            // Move generation and execution
            /** Recomputes pin directions for pieces of the side to move. */
            void
            CalcPins();
            /** Generates legal moves for one piece into @p mlist. */
            void
            GenPieceMoves(
                MoveList*  mlist,
                squareT    sq,
                SquareSet* sqset,
                bool       capturesOnly);

            /** Generates legal moves for the current side.
             *
             * The destination list is cleared first.  Generated moves are fully
             * position-resolved MoveAction records, so callers can pass them directly to
             * apply(), writeSan(), or ordering code.
             *
             * @param mlist destination list, cleared before moves are appended.
             * @param mask piece type to generate, or EMPTY for every piece type.
             * @param genType capture/non-capture filter.
             * @param maybeInCheck pass false only when the caller already knows the
             * side to move is not in check.
             */
            void
            GenerateMoves(
                MoveList* mlist,
                pieceT    mask,
                genMovesT genType,
                bool      maybeInCheck);
            /** Generates every legal move for the current side, replacing @p mlist contents. */
            void
            GenerateMoves(MoveList* mlist)
            {
                GenerateMoves(mlist, EMPTY, GEN_ALL_MOVES, true);
            }
            /** Generates legal moves matching @p genType for the current side, replacing @p mlist
             * contents. */
            void
            GenerateMoves(
                MoveList* mlist,
                genMovesT genType)
            {
                GenerateMoves(mlist, EMPTY, genType, true);
            }
            /** Generates legal captures for the current side, replacing @p mlist contents. */
            void
            GenerateCaptures(MoveList* mlist)
            {
                GenerateMoves(mlist, EMPTY, GEN_CAPTURES, true);
            }
            /** Tests a move in the current position.
             *
             * @returns 1 for a normal legal move, +2 for kingside castling, -2 for
             * queenside castling, or 0 when the move is illegal.
             */
            int
            IsLegalMove(
                squareT from,
                squareT to,
                pieceT  promo) const;

            /** Checks whether the side to move can castle on the requested side.
             *
             * @tparam check_legal when true, also require the castling right, clear
             * transit squares, and unattacked king transit squares.
             * @param king_side true for kingside castling, false for queenside.
             */
            template <bool check_legal = true>
            bool
            canCastle(bool king_side) const;

            /** Counts attacks by @p toMove on @p kingSq and optionally stores attacker squares. */
            uint
            CalcAttacks(
                colorT      toMove,
                squareT     kingSq,
                SquareList* squares) const;
            /** Evaluates recursive capture pressure on @p target for UI attack colouring. */
            int
            TreeCalcAttacks(squareT target);
            /** Counts checks on the side-to-move king. */
            uint
            CalcNumChecks() const
            {
                return CalcAttacks(1 - ToMove, GetKingSquare(), NULL);
            }
            /** Counts attacks by the enemy side on @p kingSq. */
            uint
            CalcNumChecks(squareT kingSq) const
            {
                return CalcAttacks(1 - ToMove, kingSq, NULL);
            }
            /** Counts enemy attacks on @p kingSq and optionally records checking squares. */
            uint
            CalcNumChecks(
                squareT     kingSq,
                SquareList* checkSquares) const
            {
                return CalcAttacks(1 - ToMove, kingSq, checkSquares);
            }

            /** Counts rook-like or bishop-like mobility from @p from for evaluation. */
            uint
            Mobility(
                pieceT  p,
                colorT  color,
                squareT from);
            /** Returns true when the side to move is in check. */
            bool
            IsKingInCheck()
            {
                return (CalcNumChecks() > 0);
            }
            /** Fast check test after @p sm was just applied. */
            bool
            IsKingInCheck(MoveAction const& sm);
            /** Returns true when the side to move is checkmated. */
            bool
            IsKingInMate();
            /** Returns true when the position satisfies basic legality constraints. */
            bool
            IsLegal();

            /** Returns true when the two squares could form a promotion move in either order. */
            bool
            IsPromoMove(
                squareT from,
                squareT to);

            /** Parses SAN-like or coordinate notation into a MoveSpec for this position.
             *
             * The notation is resolved against the current board state.  On failure,
             * @p spec is left unchanged.
             */
            errorT
            parseMoveSpec(
                MoveSpec&        spec,
                std::string_view notation);
            /** Parses coordinate notation into a MoveSpec.
             *
             * This accepts UCI-style coordinate moves such as @c e2e4 and @c e7e8q.
             * On failure, @p spec is left unchanged.
             *
             * @param spec destination for the parsed move request.
             * @param notation coordinate notation such as @c e2e4 or @c e7e8q.
             * @param reverse when true, also accepts the two squares in reverse order for
             * UI smart-move completion.
             */
            errorT
            readCoordinateMoveSpec(
                MoveSpec&        spec,
                std::string_view notation,
                bool             reverse);
            /** Formats @p spec as SAN in this position.
             *
             * The position is not changed.  Stored legality and ambiguity are derived
             * from the current board state, and an invalid spec returns an empty string.
             */
            std::string
            makeSan(
                MoveSpec const& spec,
                sanFlagT        flag);
            /** Resolves and applies @p spec to this position.
             *
             * The position is changed only when the move resolves successfully.
             */
            errorT
            applyMove(MoveSpec const& spec);

            /** Resolves a portable MoveSpec into a reversible MoveAction.
             *
             * Normal moves are checked for legality.  Null moves and castling specs are
             * trusted as explicit requests; use parseMoveSpec() or
             * readCoordinateMoveSpec() when input must first be validated from text.
             */
            errorT
            resolveMove(
                MoveSpec const& spec,
                MoveAction&     action) const;
            /** Applies a previously resolved action. */
            void
            apply(MoveAction const& action);
            /** Undoes a previously applied action. */
            void
            undo(MoveAction const& action);

            /** Writes SAN for a resolved action into @p s.
             *
             * The destination must have room for at least SAN_STRING_SIZE bytes.  The
             * position is temporarily advanced and then restored before the function
             * returns.
             */
            void
            writeSan(
                MoveAction const& action,
                char*             s,
                sanFlagT          flag);

            /** Applies a whitespace-separated sequence of coordinate moves.
             *
             * When @p toSAN is provided, SAN for each applied move is appended with move
             * numbers suitable for UI display.  Moves are applied incrementally; if one
             * move is invalid, earlier moves remain applied and @p toSAN may already
             * contain their SAN text.
             */
            errorT
            applyCoordinateMoves(
                const char*  moves,
                size_t       movesLen,
                std::string* toSAN = nullptr);

            // Board I/O
            /** Writes the legacy long board string representation.
             *
             * The destination must have room for 67 bytes: 64 board characters, a
             * space, side-to-move marker, and terminator.
             */
            void
            MakeLongStr(char* str) const;
            /** Reads the legacy long board string representation.
             *
             * The position is cleared before parsing.  On failure, this object may
             * contain a partially read board.
             */
            errorT
            ReadFromLongStr(const char* str);
            /** Reads FEN or EPD first-four-fields position text.
             *
             * The parser accepts ordinary FEN, Chess960 castling-file letters, and EPD
             * text that supplies only the first four FEN fields.  The position is
             * cleared before parsing; on failure, this object may contain a partially
             * read position.
             */
            errorT
            ReadFromFEN(const char* s);
            /** Reads @c startpos or a FEN/UCI-position style string.
             *
             * Accepted forms include @c "position startpos",
             * @c "position startpos moves e2e4", a raw FEN, a raw FEN followed by
             * @c "moves ...", and @c "position fen ... moves ...".  Move suffixes are
             * applied incrementally, so earlier moves remain applied if a later move is
             * invalid.
             */
            errorT
            ReadFromFENorUCI(std::string_view str);
            /** Writes the compact board representation used by UI caches.
             *
             * The destination must have room for 36 bytes including the terminator.
             */
            void
            PrintCompactStr(char* cboard) const;
            /** Writes a FEN string into @p str, bounded by @p len.
             *
             * Output is always null-terminated when @p len is non-zero.  If the buffer
             * is too small, the FEN is truncated.
             */
            void
            PrintFEN(
                char*  str,
                size_t len) const;
            /** Appends a LaTeX board diagram. */
            void
            DumpLatexBoard(
                DString* dstr,
                bool     flip);
            /** Appends a non-flipped LaTeX board diagram. */
            void
            DumpLatexBoard(DString* dstr)
            {
                DumpLatexBoard(dstr, false);
            }
            /** Appends an HTML board diagram. */
            void
            DumpHtmlBoard(
                DString*    dstr,
                uint        style,
                const char* dir,
                bool        flip);
            /** Appends a non-flipped HTML board diagram. */
            void
            DumpHtmlBoard(
                DString*    dstr,
                uint        style,
                const char* dir)
            {
                DumpHtmlBoard(dstr, style, dir, false);
            }

            // Copy, compare positions
            /** Compares this position with @p p using the legacy ordering. */
            int
            Compare(Position* p);
            /** Copies every field from @p src. */
            void
            CopyFrom(Position* src)
            {
                *this = *src;
            }

            /** Sets up a random legal position matching a material description. */
            errorT
            Random(const char* material);

        private:
            void
            setCastling(
                colorT  col,
                squareT rsq);
            void
            ClearCastling(
                colorT     col,
                castleDirT dir)
            {
                Castling &= ~(1u << castlingIdx(col, dir));
            }
            void
            ClearCastlingFlags(colorT c)
            {
                Castling &= (c == WHITE) ? 0b11111100 : 0b11110011;
            }
    };

} // namespace scid::core
