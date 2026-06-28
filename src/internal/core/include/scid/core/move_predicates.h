/** @file
 * Low-level move geometry and occupancy predicates.
 */

#pragma once

#include "scid/core/board.h"

#include <cassert>
#include <utility>

/** @namespace scid::core::move_predicates
 * Stateless helpers used by Position move validation.
 *
 * These helpers separate chess movement into four levels:
 *
 * - A valid move follows piece geometry alone, such as a bishop moving
 *   diagonally.  Validity does not require a position.
 * - An attack move is valid capture geometry plus blocker checks for sliders
 *   and pawn capture direction.  It needs only an occupancy predicate.
 * - A pseudo-legal move is an attack move, a non-capture pawn advance, or a
 *   castle candidate.
 * - A legal move is pseudo-legal, does not land on a friendly piece, and does
 *   not leave the mover's king in check.  Legal move validation belongs to
 *   Position because it needs full board state.
 */

namespace scid::core
{

    namespace move_predicates
    {

        constexpr int NSQUARES = 8;
        constexpr int kWPHomeRank = 1;
        constexpr int kBPHomeRank = NSQUARES - 2;

        /** Returns true when the two squares are a king move apart. */
        inline bool
        valid_king(
            squareT sqFrom,
            squareT sqTo)
        {
            unsigned distRank = 1 + (sqTo / NSQUARES) - (sqFrom / NSQUARES);
            unsigned distFyle = 1 + (sqTo % NSQUARES) - (sqFrom % NSQUARES);
            return distRank <= 2 && distFyle <= 2;
        }

        /** Returns true when the two squares form a knight move. */
        inline bool
        valid_knight(
            squareT sqFrom,
            squareT sqTo)
        {
            int distRank = (sqTo / NSQUARES) - (sqFrom / NSQUARES);
            int distFyle = (sqTo % NSQUARES) - (sqFrom % NSQUARES);
            int distProduct = distRank * distFyle;
            return (distProduct == 2 || distProduct == -2);
        }

        /** Returns the slider step from @p sqFrom to @p sqTo.
         *
         * @returns the signed-independent board step for a queen, rook, or bishop ray,
         * or 0 when @p pieceType cannot move along that ray.
         */
        inline int
        valid_slider(
            squareT sqFrom,
            squareT sqTo,
            pieceT pieceType)
        {
            assert(pieceType == QUEEN || pieceType == ROOK || pieceType == BISHOP);

            int distRank = (sqTo / NSQUARES) - (sqFrom / NSQUARES);
            int distFyle = (sqTo % NSQUARES) - (sqFrom % NSQUARES);

            // Make sure the direction is valid:
            int sqStep;
            bool isDiagonal = false;
            if (distRank == 0)
            {
                sqStep = 1; // horizontal
            }
            else if (distFyle == 0)
            {
                sqStep = NSQUARES; // vertical
            }
            else if (distFyle == distRank)
            {
                sqStep = NSQUARES + 1;
                isDiagonal = true;
            }
            else if (distFyle == -distRank)
            {
                sqStep = NSQUARES - 1;
                isDiagonal = true;
            }
            else
            {
                return 0;
            }
            if (pieceType == ROOK && isDiagonal)
                return 0;
            if (pieceType == BISHOP && !isDiagonal)
                return 0;

            return sqStep;
        }

        /** Returns true when a pawn of @p pieceCol attacks @p sqTo from @p sqFrom. */
        inline bool
        attack_pawn(
            squareT sqFrom,
            squareT sqTo,
            colorT pieceCol)
        {
            int distRank = (sqTo / NSQUARES) - (sqFrom / NSQUARES);
            int distFyle = (sqTo % NSQUARES) - (sqFrom % NSQUARES);
            if (pieceCol == WHITE && distRank != 1)
                return false;
            if (pieceCol == BLACK && distRank != -1)
                return false;

            return (distFyle == 1 || distFyle == -1);
        }

        /** Returns true when a slider attacks along an unobstructed ray.
         *
         * @p isOccupied is called only for squares strictly between @p sqFrom and
         * @p sqTo.
         */
        template <typename TFunc>
        bool
        attack_slider(
            squareT sqFrom,
            squareT sqTo,
            pieceT pieceType,
            TFunc isOccupied)
        {
            int sqStep = valid_slider(sqFrom, sqTo, pieceType);
            if (sqStep == 0)
                return false;

            // Make sure all the in-between squares are empty:
            if (sqFrom > sqTo)
                sqStep = -sqStep;

            for (int sq = sqFrom + sqStep; sq != sqTo; sq += sqStep)
            {
                if (isOccupied(sq))
                    return false;
            }

            return true;
        }

        /** Validates an attack move.
         *
         * @param sqFrom square of the attacking piece.
         * @param sqTo square being attacked.
         * @param pieceCol colour of the attacking piece.
         * @param pieceType type of the attacking piece.
         * @param isOccupied callable returning true when an intermediate square is
         * occupied.  It is not called for @p sqFrom or @p sqTo, so it can describe
         * either the pre-move or post-move occupancy of those endpoint squares.
         * @returns true when the piece attacks @p sqTo from @p sqFrom.
         */
        template <typename TFunc>
        bool
        attack(
            squareT sqFrom,
            squareT sqTo,
            pieceT pieceCol,
            pieceT pieceType,
            TFunc isOccupied)
        {
            switch (pieceType)
            {
                case KING:
                    return valid_king(sqFrom, sqTo);
                case KNIGHT:
                    return valid_knight(sqFrom, sqTo);
                case PAWN:
                    return attack_pawn(sqFrom, sqTo, pieceCol);
                default:
                    break;
            }
            return attack_slider(sqFrom, sqTo, pieceType, isOccupied);
        }

        /** Validates a non-capturing pawn advance.
         *
         * Single and home-rank double advances are accepted when every traversed square
         * is empty according to @p isOccupied.
         */
        template <typename TFunc>
        inline bool
        pseudo_advance_pawn(
            squareT sqFrom,
            squareT sqTo,
            colorT pieceCol,
            TFunc isOccupied)
        {
            if ((sqTo % NSQUARES) != (sqFrom % NSQUARES) // Different file
                || isOccupied(sqTo))                     // Pawns can only capture diagonally
                return false;

            int fromRank = sqFrom / NSQUARES;
            int distRank = (sqTo / NSQUARES) - fromRank;
            if (pieceCol == WHITE)
                return distRank == 1 ||
                       (distRank == 2 && fromRank == kWPHomeRank && !isOccupied(sqFrom + NSQUARES));

            return distRank == -1 ||
                   (distRank == -2 && fromRank == kBPHomeRank && !isOccupied(sqFrom - NSQUARES));
        }

        /** Validates pseudo-legal movement using piece geometry and occupancy.
         *
         * This covers attacks and non-capturing pawn advances.  Castling is handled by
         * Position because castling rights, rook identity, and attacked transit squares
         * require full position state.
         */
        template <typename TFunc>
        bool
        pseudo(
            squareT sqFrom,
            squareT sqTo,
            colorT pieceCol,
            pieceT pieceType,
            TFunc isOccupied)
        {
            // TODO: castle moves
            if (pieceType == PAWN && pseudo_advance_pawn(sqFrom, sqTo, pieceCol, isOccupied))
                return true;

            return attack(sqFrom, sqTo, pieceCol, pieceType, isOccupied);
        }

        /** Finds a possible discovered pin caused by moving from a ray.
         *
         * Given a pseudo-legal move, this returns the kind and square of the first piece
         * that may pin the mover to @p sqRay, usually the mover's king square.
         *
         * @param sqFrom origin square of the pseudo-legal move.
         * @param sqTo destination square of the pseudo-legal move.
         * @param sqRay square through which the projected ray passes.
         * @param isOccupied callable returning true when a square is occupied.
         * @returns a pair of candidate attacker type and square.  INVALID_PIECE means
         * no ray was opened.  Otherwise, the caller must inspect the returned square:
         * an enemy queen or an enemy piece matching the returned type makes the move
         * illegal.
         */
        template <typename TFunc>
        inline std::pair<pieceT, squareT>
        opens_ray(
            squareT sqFrom,
            squareT sqTo,
            squareT sqRay,
            TFunc isOccupied)
        {
            assert(sqRay != sqFrom);

            int fyleFrom = sqFrom % NSQUARES;
            int distFyle = (sqRay % NSQUARES) - fyleFrom;
            int distRank = (sqRay / NSQUARES) - (sqFrom / NSQUARES);

            // Make sure the direction is valid:
            int fyleEdge;
            int sqStep;
            pieceT pt;
            if (distFyle == 0)
            {
                sqStep = NSQUARES; // vertical
                fyleEdge = -1;
                pt = ROOK;
            }
            else
            {
                if (fyleFrom == 0 || fyleFrom == (NSQUARES - 1))
                    return {INVALID_PIECE, 0};

                if (distRank == 0)
                {
                    sqStep = 1; // horizontal
                    fyleEdge = 0;
                    pt = ROOK;
                }
                else if (distFyle == distRank)
                {
                    sqStep = NSQUARES + 1;
                    fyleEdge = 0;
                    pt = BISHOP;
                }
                else if (distFyle == -distRank)
                {
                    sqStep = NSQUARES - 1;
                    fyleEdge = NSQUARES - 1;
                    pt = BISHOP;
                }
                else
                {
                    return {INVALID_PIECE, 0};
                }
            }
            if (sqFrom > sqRay)
            {
                sqStep = -sqStep;
                fyleEdge = NSQUARES - 1 - fyleEdge;
            }

            for (int sq = sqFrom + sqStep; sq != sqRay; sq += sqStep)
            {
                if (sq == sqTo || isOccupied(sq))
                    return {INVALID_PIECE, 0};
            }

            for (int sq = sqFrom - sqStep; sq < NSQUARES * NSQUARES; sq -= sqStep)
            {
                if (sq < 0 || sq == sqTo)
                    break;

                if (isOccupied(sq))
                    return {pt, static_cast<squareT>(sq)};

                if ((sq % NSQUARES) == fyleEdge)
                    break;
            }
            return {INVALID_PIECE, 0};
        }

        /** Tests whether @p sqBlock lies between two ray-aligned squares.
         *
         * @returns true when a piece on @p sqBlock would block a queen ray from
         * @p sqFrom to @p sqTo.
         */
        inline bool
        blocks_ray(
            squareT sqFrom,
            squareT sqTo,
            squareT sqBlock)
        {
            return !move_predicates::attack_slider(
                sqFrom, sqTo, QUEEN, [&](auto sq) { return sq == sqBlock; });
        }

    } // namespace move_predicates

} // namespace scid::core
