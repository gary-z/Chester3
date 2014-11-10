#pragma once

#include "BitBoard.h"
#include "Position.h"
#include "chess_consts.h"
#include "GameConfiguration.h"
#include <cstdint>
#include "Move.h"
#include "UndoData.h"
#include "Game_move_iteration.h"

class AllMoveIteratorGenerator;
class Game {
public:
	// Initing / Configure
	static void init();
	static void reset();//Invalidates the position.
	static void configure(const GameConfiguration& conf);
	static GameConfiguration getGameConfiguration();
	static std::string getFEN();

	// Game state query
	static Turn getTurn();
	static BitBoard getPieces(Turn turn, Piece piece);
	static BitBoard getAllPieces();
	static BitBoard getPlayerPieces(Turn turn);
	static Piece getPieceAt(Position position);
	static Turn getOwnerAt(Position position);
	static bool getCanCastle(Turn turn, Side side);
	static int getEnpeasentColumn(); // 8 For cannot enpeasent
	static bool getCheck();

	static uint64_t getHash();


	// Querying moves
	static AllMoveIteratorGenerator getAllMoves();
	static AllMoveIteratorGenerator getTacticalMoves();
	static int getNumValidMoves();
	static bool areMovesAvailable();

	// Making moves
	static void makeMove(Move move);
	static Move undoMove();//returns the move we are undoing
private:
	// === FIELDS
	static Turn curTurn;
	static BitBoard WK, WQ, WR, WB, WN, WP;
	static BitBoard BK, BQ, BR, BB, BN, BP;
	static BitBoard WA, BA;
	static BitBoard ALL;
	static Piece pieces[64];


#pragma region Move Manager
	static UndoData cur;

	static int movePtr;

	static void resetMoveManager();
	static void addMove(Move move);
	static void addPawnMove(Move move);//accounts for promotions
	static void pushMove(Move move);// Called by makeMove to store this layer's info
	static void popMove();//  Called by undoMove to restore this layer's info
	static bool isDefinitelyAMoveAvailable();//Tries to find a move

	static void generateAllMoves();
	static void generateTacticalMoves();
	static void generateMovesImpl(bool tacticalOnly);
	//static void generateMovesImpl_BLACK();
	
#pragma endregion
	// === Private getters
	
	static BitBoard* s(Turn turn, Piece piece);
	static BitBoard* sp(Turn turn);
	static void setPieceAt(Position position, Piece piece);
	static void toggleBit(Turn turn, Position position, Piece piece);
	static void clearPieceAt(Position position);
	static void addPieceAt(Turn turn, Position position, Piece piece);



	// === Debug
	static void integrityCheck();
	static void assertMovesAreUnique();
	

	
	static bool posAttackedBy(Position position, Turn turn);//Does turn attack the given position?
	static bool posAttackedByJump(Position position, Turn turn);
	static bool posAttackedByLOS(Position position, Turn turn);

	//For move generation only
	static BitBoard attackedByJump(Turn turn);


	Game();
	~Game();
};

