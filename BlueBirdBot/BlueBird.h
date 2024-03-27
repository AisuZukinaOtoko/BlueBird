#pragma once
#include "../IcePickEngineManager/ChessEngineManager.h"
#include "ChessPiece.h"
#include "ChessBoard.h"

class BlueBird : public ChessEngine {
public:
	void init() override;
	void onOpponentMove(std::string) override;
private:
	ChessBoard board;
	ChessPiece pieces[32]; // 16 white pieces, 16 black pieces

	int getBoardEvaluation(ChessBoard& currentPosition);
	void updatePiecesBoard(std::string);
};