#include "MoveOrdering.h"
#include <algorithm>
#include "Game.h"
#include "AttackMap.h"
#include <iostream>
#include "Search_Killers.h"
#include "StaticExchange.h"
using namespace std;

OrderedMove::OrderedMove(Move move, int rating, OrderedMoveType type) :move(move), rating(rating), type(type) {}

bool operator<(const OrderedMove& a, const OrderedMove& b) {
	return a.rating > b.rating;
}



// ============= MOVE ORDERING
MoveOrdering::MoveOrdering() {}


namespace {
	
}
MoveOrdering::MoveOrdering(const Search_Parameters params, GameMoveIteratorGenerator gen) {
	for (Move move : gen) {
		moves.push_back(order(params, move));
	}
	sort(moves.begin(), moves.end());

	//cout << "===" << endl;
	//cout << Game::getFEN() << endl;
	//for (auto move : moves) {
	//	cout << move.move.str() << ' ' << move.rating << endl;


	//}
	//system("pause");
}


MoveOrdering::~MoveOrdering() {}


OrderedMove MoveOrdering::order(const Search_Parameters params,const Move move) {
	int rating = 0;
	OrderedMoveType type = OrderedMoveType::NONE;
	
	if (move.isTactical()) {
		int see = AttackMap::SEE(move);
		if (see > 0) {
			rating = 50000 + see * 100 - StaticExchange::PieceValue(move.getPiece());
			type = OrderedMoveType::WINNING_CAPTURE;
		} else if (see == 0) {
			type =  OrderedMoveType::EVEN_CAPTURE;
			rating = 30000 + see;
		} else {
			type = OrderedMoveType::LOSING_CAPTURE;
			rating = 20000 + see;
		}
	} else if (Search_Killers::isKiller(params, move)) {
		type = OrderedMoveType::KILLER;
		rating = 25000;

	}

	return OrderedMove(move, rating, OrderedMoveType::NONE);
}

std::vector<OrderedMove>::const_iterator MoveOrdering::begin() const {
	return moves.begin();
}

std::vector<OrderedMove>::const_iterator MoveOrdering::end() const {
	return moves.end();
}



