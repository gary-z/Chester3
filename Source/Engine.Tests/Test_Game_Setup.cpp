#include "stdafx.h"
#include "CppUnitTest.h"
#include "Game.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;



namespace EngineTests {
	TEST_CLASS(Test_Game_Setup) {
public:
	TEST_CLASS_INITIALIZE(CallGameInit) {
		Game::init();
	}

	TEST_METHOD_INITIALIZE(CallGameReset) {
		Game::reset();
	}

	TEST_METHOD(Game_Result) {
		Assert::AreEqual(GameResult::NONE, Game::getState());

		Game::configure(GameConfiguration("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
		Assert::AreEqual(Turn::WHITE, Game::getOwnerAt(Position("a1")));
		Assert::AreEqual(GameResult::IN_PROGRESS, Game::getState());

		Assert::AreEqual(Turn::WHITE, GameConfiguration::extractFromGame().getOwnerAt(Position("a1")));
	}


	void testConvert(std::string FEN) {
		GameConfiguration conf(FEN);
		Game::configure(conf);
		Assert::AreEqual(conf.str(), GameConfiguration::extractFromGame().str());
	}
	TEST_METHOD(Game_Configure_and_Extract_Configuration) {
		testConvert("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
		testConvert("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
		testConvert("rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2");
		testConvert("rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2");
		testConvert("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
		testConvert("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
		testConvert("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
		testConvert("r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1");
	}
	};


}