#include "Game.h"
#include <crtdbg.h>
#include <iostream>

using namespace std;
void Game::integrityCheck() {

#ifdef _DEBUG

	_ASSERTE(ALL == (WA ^ BA));
	_ASSERTE((WA & BA) == BitBoard::EMPTY());

	FOR_TURN(turn) {
		BitBoard turnAll = BitBoard::EMPTY();
		FOR_PIECE_ALL(piece) {
			turnAll ^= *s(turn, piece);
		}
	}

	FOR_PIECE_ALL(piece) {
		BitBoard pieceAll = BitBoard::EMPTY();
		FOR_POSITION_64(pos) {
			if (getPieceAt(pos) == piece) {
				pieceAll ^= pos.ToSingletonBoard();
			}

		}

		_ASSERTE((*s(Turn::WHITE, piece) ^ *s(Turn::BLACK, piece)) == pieceAll);

		FOR_TURN(turn) {
			BitBoard turnPieces = BitBoard::EMPTY();
			FOR_POSITION_64(pos) {
				if (getPieceAt(pos) == piece && getOwnerAt(pos)==turn) {
					turnPieces ^= pos.ToSingletonBoard();
				}
			}
			_ASSERTE(*s(turn, piece) == turnPieces);
		}
	}

	
	_ASSERTE(hash.toInt64() == GameHash(GameConfiguration::extractFromGame()).toInt64());

#endif
}

void Game::assertMovesAreUnique() {
#ifdef _DEBUG
	generateMoves();
	for (int i = 0; i < getNumValidMoves(); ++i) {
		Move m1 = getMove(i);
		for (int j = 0; j < getNumValidMoves(); ++j) {
			if (j == i) {
				continue;
			}
			Move m2 = getMove(j);

			if (!m1.isPromotion()) {
				_ASSERTE(!(m1.getFrom() == m2.getFrom() && m1.getTo() == m2.getTo()));
			}
		}
	}

#endif
}

std::string Game::boardRepr() {
	return GameConfiguration::extractFromGame().str();
}