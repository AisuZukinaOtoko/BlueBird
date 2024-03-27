#pragma once
#include "../Vendor/SDL/include/SDL.h"
#include <string>
class ChessEngineManager;


class ChessEngine {
public:
	virtual void init(){};
	std::string myTeam; // "black" or "white"
	// override this function. do all calculations then call ChessEngine::makeMove
	virtual void onOpponentMove(std::string boardPosition) = 0;
	const char* getName() const { return name; };
	void makeMove(std::string newBoardPosition);
	void bindManager(ChessEngineManager* parentManager);
protected:
	const char* name = "My Chess Bot";
private:
	ChessEngineManager* parentManager = nullptr;
};



namespace ChessEngineManagerUtil {
	struct Move {
		char moveIndex;
	};

	struct Settings {
		Uint8 bColour[3] = {0,0,0};
		Uint8 pColour[3] = { 0,0,0};
		Uint8 sColour[3] = { 0,0,0};
		Uint32 wTime = 0;
		Uint32 bTime = 0;
	};

	class PieceInfo {
	public:
		SDL_Rect sourceRect;
		char pieceFEN;
		Move legalMoves[30];
		short movePattern[8];
		short movePatternCount = 0;
		bool firstMove = true;
	private:
	};

	struct Piece {
		char pieceFEN = '0';
		unsigned char lastMoveNum = 0;
		unsigned char moveCount = 0;
	};
}

class ChessEngineManager {
public:
	void engineInit(ChessEngine* bot);
	void engineRun();
	void makeMove(std::string move);
private:
	ChessEngine* childEngine = nullptr;
	int windowWidth = 600, windowHeight = 600;
	bool managerRunning = false;
	bool isTurn = false;
	bool isPromotion = false;
	int currentMoveNum = 1;
	std::string friendlyPieces = "black";
	std::string engineMode;
	enum PHASE {BROADCAST = 0, SETUP, CONNECTED};
	PHASE initPhase = BROADCAST;
	void establishConnection(ChessEngine* bot);

	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Texture* chessPieceSprite = nullptr;
	SDL_Texture* promotionBackgroundSprite = nullptr;
	ChessEngineManagerUtil::Settings settings;
	void initSettings();
	void PvComRun();
	void ComvComRun();

	std::string positionFEN;
	ChessEngineManagerUtil::PieceInfo pieceSourceRects[12];
	//char chessBoard[8][8];
	ChessEngineManagerUtil::Piece chessBoard[8][8];
	void windowEventHandler();
	void render();
	int getCellIndex();

	//char pickUpPiece = '0'; // random character should not be a valid piece
	ChessEngineManagerUtil::Piece pickUpPiece; // random character should not be a valid piece
	int pickUpIndex = -1; // Prevent placing in the same spot you pick up
	int putDownIndex = -1;
	int promotionCell = -1;
	SDL_Rect promotionBack;
	SDL_Rect promotionChoices;
	void onPickUp(int cellIndex);
	void onPlace(int cellIndex);
	void cancelPlace();
	void autoQueen();

	std::string boardToFEN(ChessEngineManagerUtil::Piece board[8][8]);
	void FENToBoard(std::string positionFEN, ChessEngineManagerUtil::Piece board[8][8]);

	// returns 0 if vacant. 1 if enemy piece. 2 if friendly piece
	int cellOccupied(int cellIndex);
	bool xPathBlocked(int index1, int index2);
	bool yPathBlocked(int index1, int index2);
	bool DiagonalPathBlocked(int index1, int index2);
	void promotePawn(); // promotes pawn
	bool capturePromote(int& cellIndex); // checks for captures or promotions
	void onMyMove(std::string move);
};