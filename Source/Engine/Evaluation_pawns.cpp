#include "Evaluation.h"
#include "Game.h"
#include "AttackFields.h"
#include "AttackMap.h"

namespace {
	const char formation_1[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		16, 16, 16, 16, 16, 16, 16, 16,
		13, 14, 16, 16, 16, 16, 14, 13,
		8, 10, 13, 13, 13, 13, 10, 8,
		5, 7, 10, 10, 10, 10, 7, 5,
		2, 2, 2, 2, 2, 2, 2, 2,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	};

	const char formation_2[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		10, 13, 16, 16, 16, 16, 13, 10,
		7, 10, 13, 13, 13, 13, 10, 7,
		5, 7, 8, 8, 8, 8, 7, 5,
		3, 5, 7, 7, 7, 7, 5, 3,
		2, 2, 2, 2, 2, 2, 2, 2,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	};

	const char weakpawn_1[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		5, 5, 5, 5, 5, 5, 5, 5,
		3, 3, 3, 3, 3, 3, 3, 3,
		3, 5, 5, 3, 3, 5, 5, 3,
		2, 5, 5, 3, 3, 5, 5, 2,
		0, 3, 5, 7, 7, 5, 3, 0,
		0, 2, 3, 7, 7, 3, 2, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	};

	const char weakpawn_2[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		3, 5, 5, 5, 5, 5, 5, 3,
		3, 7, 13, 16, 16, 13, 7, 3,
		3, 5, 10, 10, 10, 10, 5, 3,
		3, 5, 10, 10, 10, 10, 5, 3,
		3, 5, 10, 13, 13, 10, 5, 3,
		3, 3, 7, 13, 13, 7, 3, 3,
		0, 0, 0, 0, 0, 0, 0, 0,
	};

	// 
	const char importance[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2,
		0, 0, 0, 2, 2, 0, 0, 0,
		0, 0, 0, 2, 2, 0, 0, 0,
		0, 1, 2, 2, 2, 2, 1, 0,
		0, 1, 2, 2, 2, 2, 1, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
}
int Evaluation::pawnStructureScore(Turn turn) {
	int res = 0;
	const Turn other = !turn;
	const BitBoard MP = Game::getPieces(turn, Piece::PAWN());
	const BitBoard TP = Game::getPieces(other, Piece::PAWN());
	const BitBoard myPawnCoverage = AttackFields::pawnTargs(MP, turn);
	const BitBoard theirPawnCoverage = AttackFields::pawnTargs(TP, other);

	// Stacked pawns
	FOR_BIT(pawn, MP) {
		const Position pos = pawn.ToPosition();
		const BitBoard colBits = BitBoard::colBits(pos.col());
		int nPawnsInCol = (colBits&MP).count();
		_ASSERTE(nPawnsInCol >= 1);
		if (nPawnsInCol == 1) { // not stacked, skip
			continue;
		}
		res -= (nPawnsInCol - 1) * 20; // penalty based on stackage factor

		// No friendly pawns on adjacent files
		if ((MP&(colBits.shiftLeft() | colBits.shiftRight())).isEmpty()) {
			res -= 20;
		}

		if ((colBits&TP).isEmpty()) { // On half open file
			res -= 10;
		}
	}

	BitBoard weakPawns = MP; // not defended or easily defendable

	const BitBoard sideBySide = (weakPawns.shiftForward(turn)&myPawnCoverage).shiftBackward(turn);
	weakPawns ^= sideBySide;
	FOR_BIT(pawn, sideBySide) {
		res += formation_1[pawn.ToPosition().perspective(turn).index()];
	}
	_ASSERTE(sideBySide == ((MP.shiftLeft()&MP) | (MP.shiftRight()&MP)));

	const BitBoard directlyProtected = weakPawns&myPawnCoverage;
	weakPawns ^= directlyProtected;

	const BitBoard aBitAhead = (weakPawns.shiftBackward(turn)&myPawnCoverage).shiftForward(turn);
	weakPawns ^= aBitAhead;
	FOR_BIT(pawn, aBitAhead | directlyProtected) {
		res += formation_2[pawn.ToPosition().perspective(turn).index()];
	}

	FOR_BIT(pawn, weakPawns) {
		const Position pos = pawn.ToPosition().perspective(turn);
		res -= weakpawn_1[pos.index()];
		if ((BitBoard::colBits(pos.col()) & TP).isEmpty()) { // half open file
			res -= weakpawn_2[pos.index()];
		}
	}



	_ASSERTE(MP == (weakPawns ^ sideBySide ^ directlyProtected ^ aBitAhead));

	// Pressure on unprotected pawns. Exclude pawns that attack their pawn
	FOR_BIT(pawn, MP &~(myPawnCoverage | theirPawnCoverage)) {
		const Position pos = pawn.ToPosition();
		const AttackPattern defenders = AttackMap::getAttackPattern(turn, pos);
		const AttackPattern attackers = AttackMap::getAttackPattern(other, pos);

		if (attackers.isEmpty() ||
			defenders.getSmallestPiece() < attackers.getSmallestPiece()) {
			continue;
		}
		int penalty = importance[pos.perspective(turn).index()];
		penalty <<= attackers.getCount();

#pragma warning (disable: 4244)//loss of precison
		penalty += Evaluation::lateness() * penalty;
		penalty *= 0.4; // scale for binary
#pragma warning (default: 4244)

		res -= penalty;
	}

	return res;
}

Evaluation::PassedPawnResult::PassedPawnResult() {
	passedPawns = BitBoard::EMPTY();
	score = 0;
}

namespace {
	const char passedPawnValue[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
	   20,20,20,20,20,20,20,20,
	   16,16,16,16,16,16,16,16,
	   11,11,11,11,11,11,11,11,
		5, 5, 5, 5, 5, 5, 5, 5,
		3, 3, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 3, 3,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
}

Evaluation::PassedPawnResult Evaluation::passedPawnEvaluation(Turn turn) {
	PassedPawnResult res;
	const Turn other = !turn;
	const BitBoard MP = Game::getPieces(turn, Piece::PAWN());
	const BitBoard TP = Game::getPieces(other, Piece::PAWN());
	const BitBoard myPawnCoverage = AttackFields::pawnTargs(MP, turn);

	// Identify passed pawns.
	FOR_BIT(pawn, MP) {
		const Position pos = pawn.ToPosition();
		if ((Evaluation::opposingPawns(pos, turn)&TP).isEmpty()) {
			res.passedPawns |= pawn;
		}
	}

	FOR_BIT(pawn, res.passedPawns) {
		const Position pos = pawn.ToPosition();
#pragma warning (disable : 4244) //loss of precison
		const int baseScore = (1.0 + lateness())* passedPawnValue[pos.perspective(turn).index()];
#pragma warning (default : 4244)

		res.score += baseScore;

		// connected with friendly pawn side by side or directly
		if ((pawn&myPawnCoverage).isNotEmpty() || (pawn.shiftForward(turn)&myPawnCoverage).isNotEmpty()) {
			res.score += baseScore;
		}

		
		const Position pos_1 = pos.shiftForward(turn); // 1 square forward
		// Check how far can this pawn get
		if (Game::getPieceAt(pos_1) == Piece::EMPTY()) {// in front is empty

			// === 1 square ahead check
			const AttackPattern next_1_defenders = AttackMap::getAttackPattern(turn, pos_1);
			const AttackPattern next_1_attackers = AttackMap::getAttackPattern(other, pos_1);

			// Next square is attacked, but not defended
			if (!next_1_attackers.isEmpty() && next_1_defenders.isEmpty()) {
				continue;
			}
			res.score += baseScore;

			// reached end of board
			if (pos_1.row() == 0 || pos_1.row() == 7) {
				res.score += 2 * baseScore;
				continue;
			}

			// === 2 squares ahead check
			const Position pos_2 = pos_1.shiftForward(turn);
			if (Game::getPieceAt(pos_2) != Piece::EMPTY()) {
				continue;
			}

			const AttackPattern next_2_defenders = AttackMap::getAttackPattern(turn, pos_2);
			const AttackPattern next_2_attackers = AttackMap::getAttackPattern(other, pos_2);
			// Next square is attacked, but not defended
			if (next_2_attackers.isNotEmpty() && next_2_defenders.isEmpty()) {
				continue;
			}
			res.score += baseScore;

			// reached end of board
			if (pos_2.row() == 0 || pos_2.row() == 7) {
				res.score += baseScore;
				continue;
			}

			const Position pos_3 = pos_2.shiftForward(turn);
			if (Game::getPieceAt(pos_3) != Piece::EMPTY()) {
				continue;
			}

			const AttackPattern next_3_defenders = AttackMap::getAttackPattern(turn, pos_3);
			const AttackPattern next_3_attackers = AttackMap::getAttackPattern(other, pos_3);
			if (next_3_attackers.isNotEmpty() && next_3_defenders.isEmpty()) {
				continue;
			}

			res.score += baseScore;

		} else { // Square in front is occupied
			if (Game::getOwnerAt(pos_1) == turn) {// We occupy the square in front
				res.score += baseScore;
			}

			if (AttackMap::getAttackPattern(turn, pos_1).getCount() >= 2) { //square in front is covered by 2 of our pieces
				res.score += baseScore;
			}

			// square in front is defended
			const AttackPattern next_1_defenders = AttackMap::getAttackPattern(turn, pos_1);
			if (next_1_defenders.isNotEmpty()) {
				res.score += baseScore;
			}

		}
	}

	return res;
}

int Evaluation::pawns(Turn turn) {
	return pawnStructureScore(turn) + passedPawnEvaluation(turn).score;
}

int Evaluation::DEBUG_pawnFormation(Turn turn) {
	return pawnStructureScore(turn);
}

BitBoard Evaluation::DEBUG_passedPawns(Turn turn) {
	return passedPawnEvaluation(turn).passedPawns;
}