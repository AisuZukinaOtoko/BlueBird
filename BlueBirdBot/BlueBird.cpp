#include <iostream>
#include "BlueBird.h"
#include "Assert.h"


void BlueBird::init() {
	std::string pieceString = "PPPPPPPPRNBQKBNRpppppppprnbqkbnr";
	for (int i = 0; i < 32; i++) {
		pieces[i].pieceInit(pieceString[i]);
	}
}

void BlueBird::onOpponentMove(std::string boardPosition) {
	updatePiecesBoard(boardPosition);
							
	board.FENtoBoard(boardPosition);
	std::cout << board.getFEN() << std::endl;
	std::cout << board;
}

int BlueBird::getBoardEvaluation(ChessBoard& currentPosition) {
	return 0;
}

void BlueBird::updatePiecesBoard(std::string position) {

}