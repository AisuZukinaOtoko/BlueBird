#include "ChessEngineManager.h"
#include "../Vendor/SDL/include/SDL_image.h"
#include "../Vendor/FileWatch.hpp"
#include "../Vendor/json.hpp"
#include <iostream>
#include <fstream>
#include <math.h>
#include <Windows.h>

void logError(std::string error) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 4);
	std::cout << error << std::endl;
	SetConsoleTextAttribute(hConsole, 7);
}

void ChessEngine::makeMove(std::string move) {
	parentManager->makeMove(move);
}

void ChessEngine::bindManager(ChessEngineManager* manager) {
	this->parentManager = manager;
}

SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path) {
	SDL_Surface* imageSurface = IMG_Load(path);
	if (!imageSurface)
		std::cout << "Error loading sprite\n" << SDL_GetError() << std::endl;

	SDL_Texture* imageTexture = SDL_CreateTextureFromSurface(renderer, imageSurface);

	if (!imageTexture)
		std::cout << "Error loading sprite\n" << SDL_GetError() << std::endl;

	SDL_FreeSurface(imageSurface);
	return imageTexture;
}

static bool isFriendlyPiece(std::string& friendlyPieces, char pieceFEN) {
	if (friendlyPieces == "white" && pieceFEN > 'Z') {
		return false;
	}
	else if (friendlyPieces == "black" && pieceFEN < 'a') {
		return false;
	}

	return true;
}

void ChessEngineManager::initSettings() {
	using nlohmann::json;

	std::ifstream jsonFileStream("settings.json");
	json settingsData = json::parse(jsonFileStream);

	json bColourArray = settingsData["Background-Colour"];
	int index = 0;
	for (auto iterator = bColourArray.begin(); iterator != bColourArray.end(); iterator++) {
		settings.bColour[index] = iterator.value();
		index++;
	}

	json pColourArray = settingsData["Primary-Colour"];
	index = 0;
	for (auto iterator = pColourArray.begin(); iterator != pColourArray.end(); iterator++) {
		settings.pColour[index] = iterator.value();
		index++;
	}

	json sColourArray = settingsData["Secondary-Colour"];
	index = 0;
	for (auto iterator = sColourArray.begin(); iterator != sColourArray.end(); iterator++) {
		settings.sColour[index] = iterator.value();
		index++;
	}

	engineMode = settingsData["Mode"];
	int randomSeed = settingsData["Random-Seed"];
	std::srand(randomSeed);

	jsonFileStream.close();
}

void ChessEngineManager::establishConnection(ChessEngine* bot) {
	onMyMove("black ok");

	{ // Initial broadcast phase
		filewatch::FileWatch<std::wstring> watch(
			L"IcePickEngineManager/Notify.txt",
			[&](const std::wstring& path, const filewatch::Event change_type) {
				std::string fileInput;
				std::ifstream inFile("Move.txt");
				std::getline(inFile, fileInput);
				std::cout << "Input: " << fileInput << std::endl;

				if (fileInput == "black ok") {
					friendlyPieces = "white";
					isTurn = true;
					initPhase = SETUP;
					bot->myTeam = friendlyPieces;
					bot->init();
				}
				else {
					//if (initPhase == BROADCAST) {
					friendlyPieces = "black";
					isTurn = true;
					bot->myTeam = friendlyPieces;
					bot->init();
					//}
					initPhase = CONNECTED;
					//std::cout << "Chess bot connected: ";
					std::cout << friendlyPieces << std::endl;
				}
			}
		);


		while (initPhase == BROADCAST) {
			SDL_Delay(30);
		}
	}
	onMyMove(bot->getName());

	if (initPhase == SETUP) {
		filewatch::FileWatch<std::wstring> watch(
			L"IcePickEngineManager/Notify.txt",
			[&](const std::wstring& path, const filewatch::Event change_type) {
				std::string fileInput;
				std::ifstream inFile("Move.txt");
				std::getline(inFile, fileInput);
				std::cout << "Other bot name: " << fileInput << std::endl;

				initPhase = CONNECTED;
				
				std::cout << friendlyPieces << std::endl;
			}
		);

		while (initPhase == SETUP) {
			SDL_Delay(30);
		}
	}

	if (initPhase == CONNECTED) {
		std::cout << "Chess bot connected: ";
	}
}

void ChessEngineManager::onMyMove(std::string move) {
	std::ofstream outFile("Move.txt");
	outFile << move;
	outFile.close();
	outFile.open("IcePickEngineManager/Notify.txt");
	outFile.close();
}

int ChessEngineManager::getCellIndex() {
	int x, y;
	SDL_GetMouseState(&x, &y);
	int boardDimension = (windowWidth > windowHeight) ? windowHeight : windowWidth; // min{windowWidth, windowHeight}
	int gridCellDimension = boardDimension / 8;
	int cellIndex = (y / gridCellDimension) * 8 + (x / gridCellDimension);
	return cellIndex;
}

void ChessEngineManager::makeMove(std::string move) {
	if (move.length() == 4) {
		int pUI = (7 - (move[1] - '1')) * 8 + (move[0] - 'a');
		int pDI = (7 - (move[3] - '1')) * 8 + (move[2] - 'a');
		if (isFriendlyPiece(friendlyPieces, chessBoard[pUI / 8][pUI % 8].pieceFEN) == false) {
			std::string error = "Move: " + move + " was invalid";
			logError("That's not your piece...");
			logError(error);
			return;
		}
		onPickUp(pUI);
		onPlace(pDI);
	}
	else if (move.length() == 5) { // pawn promotion
		if (move[4] < 'A') {
			std::string error = "Move: " + move + " was invalid";
			logError(error);
			return;
		}
		std::cout << "Pawn promotion\n";
	}
	else {
		std::string error = "Move: " + move + " was invalid";
		logError(error);
	}
}

std::string ChessEngineManager::boardToFEN(ChessEngineManagerUtil::Piece board[8][8]) {
	std::string FEN = "";
	for (int i = 0; i < 8; i++) {
		int gap = 0;
		for (int j = 0; j < 8; j++) {
			if (board[i][j].pieceFEN != '0' && gap) {
				FEN += std::to_string(gap) + board[i][j].pieceFEN;
				gap = 0;
			}
			else if (board[i][j].pieceFEN != '0') {
				FEN += board[i][j].pieceFEN;
			}
			else { // '0'
				gap++;
			}
		}
		if (gap)
			FEN += std::to_string(gap);
		if (i < 7)
			FEN += '/';
	}
	return FEN;
}

void ChessEngineManager::FENToBoard(std::string FEN, ChessEngineManagerUtil::Piece board[8][8]) {
	int x = 0, y = 0;
	for (int i = 0; i < FEN.length(); i++) {
		if (FEN[i] == '/') {
			y++;
			x = 0;
			continue;
		}
		else if (FEN[i] > 48 && FEN[i] < 57) { // a number
			for (int k = 0; k < FEN[i] - '0'; k++) {
				board[y][x + k].pieceFEN = '0';
			}
			x += FEN[i] - '0';
			continue;
		}
		board[y][x].pieceFEN = FEN[i];
		x++;
	}
}

void ChessEngineManager::engineInit(ChessEngine* bot) {
	initSettings();
	if (engineMode == "com-com") {
		std::cout << "Connecting...\n";
		establishConnection(bot);
	}
	else {
		bot->myTeam = friendlyPieces;
		bot->init();
		friendlyPieces = "black";
	}
	
	bot->bindManager(this);
	childEngine = bot;
	managerRunning = true;

	positionFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
	FENToBoard(positionFEN, chessBoard);

	if (friendlyPieces == "black" && engineMode == "com-com") {
		return;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
		window = SDL_CreateWindow(bot->getName(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_RESIZABLE);
		if (!window) {
			managerRunning = false;
			return;
		}
		renderer = SDL_CreateRenderer(window, -1, 0);
		chessPieceSprite = loadTexture(renderer, "IcePickEngineManager/Assets/ChessPieces.png");
		promotionBackgroundSprite = loadTexture(renderer, "IcePickEngineManager/Assets/promotion background.png");
		int width, height;
	}


	// Initialise chess piece information
	pieceSourceRects[0].sourceRect = {0, 0, 200, 200}; pieceSourceRects[0].pieceFEN = 'K';
	pieceSourceRects[1].sourceRect = { 200, 0, 200, 200 }; pieceSourceRects[1].pieceFEN = 'Q';
	pieceSourceRects[2].sourceRect = { 400, 0, 200, 200 }; pieceSourceRects[2].pieceFEN = 'B';
	pieceSourceRects[3].sourceRect = { 600, 0, 200, 200 }; pieceSourceRects[3].pieceFEN = 'N';
	pieceSourceRects[4].sourceRect = { 800, 0, 200, 200 }; pieceSourceRects[4].pieceFEN = 'R';
	pieceSourceRects[5].sourceRect = { 1000, 0, 200, 200 }; pieceSourceRects[5].pieceFEN = 'P';
	pieceSourceRects[6].sourceRect = { 0, 200, 200, 200 }; pieceSourceRects[6].pieceFEN = 'k';
	pieceSourceRects[7].sourceRect = { 200, 200, 200, 200 }; pieceSourceRects[7].pieceFEN = 'q';
	pieceSourceRects[8].sourceRect = { 400, 200, 200, 200 }; pieceSourceRects[8].pieceFEN = 'b';
	pieceSourceRects[9].sourceRect = { 600, 200, 200, 200 }; pieceSourceRects[9].pieceFEN = 'n';
	pieceSourceRects[10].sourceRect = { 800, 200, 200, 200 }; pieceSourceRects[10].pieceFEN = 'r';
	pieceSourceRects[11].sourceRect = { 1000, 200, 200, 200 }; pieceSourceRects[11].pieceFEN = 'p';

	
}

void ChessEngineManager::PvComRun() {
	filewatch::FileWatch<std::wstring> watch(
		L"IcePickEngineManager/Notify.txt",
		[&](const std::wstring& path, const filewatch::Event change_type) {
			if (!isTurn)
				return;
			std::string move;
			std::ifstream inFile("Move.txt");
			inFile >> move;
			childEngine->onOpponentMove(move);
		}
	);

	while (managerRunning) {
		windowEventHandler();
		render();
		SDL_Delay(30);
	}
	SDL_Quit();
}

void ChessEngineManager::ComvComRun() {
	bool awaitingResponse = (friendlyPieces == "black");
	std::string move = positionFEN;
	while (managerRunning) {
		{
			filewatch::FileWatch<std::wstring> watch(
			L"IcePickEngineManager/Notify.txt",
			[&](const std::wstring& path, const filewatch::Event change_type) {
				std::ifstream inFile("Move.txt");
				inFile >> move;
				awaitingResponse = false;
				positionFEN = move;
				FENToBoard(positionFEN, chessBoard);
			}
			);

			while (awaitingResponse && managerRunning) {
				if (friendlyPieces == "white") {
					windowEventHandler();
					render();
				}
				SDL_Delay(30);
			}
		}

		if (friendlyPieces == "white") { // Render black's move
			render();
		}
		childEngine->onOpponentMove(move);
		awaitingResponse = true;
	}
	SDL_Quit();

}

void ChessEngineManager::engineRun() {
	if (engineMode == "com-com") {
		ComvComRun();
	}
	else {
		PvComRun();
	}
}

void ChessEngineManager::windowEventHandler() {
	SDL_Event sdlEvent;
	while (SDL_PollEvent(&sdlEvent)) {
		switch (sdlEvent.type) {
		case SDL_WINDOWEVENT:
			SDL_GetWindowSize(window, &windowWidth, &windowHeight);
			break;

		case SDL_MOUSEBUTTONDOWN:
			if (engineMode == "com-com")
				return;

			switch (sdlEvent.button.button) {
			case SDL_BUTTON_LEFT:
				if (!isPromotion) {
					int cellIndex = getCellIndex();
					if (isTurn && !isFriendlyPiece(friendlyPieces, chessBoard[cellIndex / 8][cellIndex % 8].pieceFEN)) {
						std::string error = friendlyPieces + " to move.";
						logError(error);
						break;
					}
					if (!isTurn && isFriendlyPiece(friendlyPieces, chessBoard[cellIndex / 8][cellIndex % 8].pieceFEN)) {
						std::string error = "Not " + friendlyPieces + " to move.";
						logError(error);
						break;
					}

					onPickUp(getCellIndex());
				}
				break;
			case SDL_BUTTON_RIGHT:
				pickUpPiece.pieceFEN = '0';
				FENToBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR", chessBoard);
				positionFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";

				for (int i = 0; i < 8; i++) {
					for (int j = 0; j < 8; j++)
						chessBoard[i][j].lastMoveNum = 0;
				}
				break;
			}
			break;

		case SDL_MOUSEBUTTONUP:
			if (engineMode == "com-com")
				return;

			switch (sdlEvent.button.button) {
			case SDL_BUTTON_LEFT:
				if (isPromotion)
					promotePawn();
				else {
					onPlace(getCellIndex());
				}
				break;
			}
			break;

		case SDL_QUIT:
			managerRunning = false;
			break;
		}
	}
}

void ChessEngineManager::render() {
	SDL_SetRenderDrawColor(renderer, settings.bColour[0], settings.bColour[1], settings.bColour[2], 255);
	//std::cout << +settings.bColour[0] << " " << +settings.bColour[1] << " " << +settings.bColour[2] << std::endl;
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, settings.pColour[0], settings.pColour[1], settings.pColour[2], 255);
	int boardDimension = (windowWidth > windowHeight) ? windowHeight : windowWidth; // min{windowWidth, windowHeight}

	SDL_Rect board = { 0, 0, boardDimension, boardDimension };
	SDL_RenderFillRect(renderer, &board);

	SDL_SetRenderDrawColor(renderer, settings.sColour[0], settings.sColour[1], settings.sColour[2], 255);
	int gridCellDimension = board.w / 8;
	uint64_t gridVisualisation = 0b0101010110101010010101011010101001010101101010100101010110101010;
	uint64_t runningValue = 0b1;
	for (int i = 0; i < 64; i++) {
		if (gridVisualisation & runningValue << i)
		{
			SDL_Rect gridCell = { (i % 8) * gridCellDimension, (i / 8) * gridCellDimension, gridCellDimension, gridCellDimension };
			SDL_RenderFillRect(renderer, &gridCell);
		}
	}

	// Visualising the board using the FEN string
	SDL_Rect pieceDest = {0, 0, gridCellDimension, gridCellDimension};
	for (int i = 0; i < positionFEN.size(); i++) {
		if (positionFEN[i] == '/') {
			pieceDest.y += gridCellDimension;
			pieceDest.x = 0;
			continue;
		}
		else if (positionFEN[i] > 48 && positionFEN[i] < 57) { // a number
			pieceDest.x += (positionFEN[i] - '0') * gridCellDimension;
			continue;
		}
		for (int j = 0; j < 12; j++) {
			if (pieceSourceRects[j].pieceFEN == positionFEN[i]) {
				SDL_RenderCopy(renderer, chessPieceSprite, &pieceSourceRects[j].sourceRect, &pieceDest);
				pieceDest.x += gridCellDimension;
				break;
			}
		}
	}

	//SDL_RenderCopy(renderer, frameSprite, nullptr, &board);

	// Show picking up a piece
	if (pickUpPiece.pieceFEN != '0') {
		int x, y;
		SDL_GetMouseState(&x, &y);
		for (int j = 0; j < 12; j++) {
			if (pieceSourceRects[j].pieceFEN == pickUpPiece.pieceFEN) {
				SDL_Rect pickUpDest = { x - gridCellDimension / 2, y - gridCellDimension / 2, gridCellDimension, gridCellDimension };
				SDL_RenderCopy(renderer, chessPieceSprite, &pieceSourceRects[j].sourceRect, &pickUpDest);
				pieceDest.x += gridCellDimension;
				break;
			}
		}
	}

	// Show the options when a pawn can promote
	if (isPromotion) {
		SDL_RenderCopy(renderer, promotionBackgroundSprite, nullptr, &promotionBack);
		SDL_Rect src = {200, 0, 800, 200};
		src.y = (promotionCell < 8) ? 0 : 200;
		SDL_RenderCopy(renderer, chessPieceSprite, &src, &promotionChoices);
	}

	SDL_RenderPresent(renderer);
}

void ChessEngineManager::onPickUp(int cellIndex) {
	int runningCount = 0;
	pickUpIndex = cellIndex;
	pickUpPiece = chessBoard[cellIndex / 8][cellIndex % 8];
	chessBoard[cellIndex / 8][cellIndex % 8].pieceFEN = '0';
	positionFEN = boardToFEN(chessBoard);
}

void ChessEngineManager::onPlace(int cellIndex) {
	if (pickUpPiece.pieceFEN == '0') {
		cancelPlace();
		return;
	}
	putDownIndex = cellIndex;

	bool specialMove = capturePromote(cellIndex);

	if (cellIndex == pickUpIndex || cellOccupied(cellIndex) == 2 && !specialMove) {
		cancelPlace();
		pickUpPiece.pieceFEN = '0';
		return;
	}

	if (!specialMove) {
		chessBoard[cellIndex / 8][cellIndex % 8] = pickUpPiece;
		chessBoard[cellIndex / 8][cellIndex % 8].lastMoveNum = currentMoveNum;
		chessBoard[cellIndex / 8][cellIndex % 8].moveCount++;
	}
	
	pickUpPiece.pieceFEN = '0';
	positionFEN = boardToFEN(chessBoard);

	if (isPromotion && engineMode == "com-com") {
		autoQueen();
	}
	else if (isPromotion)
		return;

	currentMoveNum++;
	isTurn = !isTurn;

	char move[5] = { '\0' };
	move[0] = (pickUpIndex % 8 + 'a');
	move[1] = (7 - pickUpIndex / 8 + '1');
	move[2] = (putDownIndex % 8 + 'a');
	move[3] = (7 - putDownIndex / 8 + '1');
	onMyMove(positionFEN);
}

void ChessEngineManager::cancelPlace() {
	chessBoard[pickUpIndex / 8][pickUpIndex % 8] = pickUpPiece;
	positionFEN = boardToFEN(chessBoard);
	logError("Move was cancelled.");
}

int ChessEngineManager::cellOccupied(int cellIndex) {
	char cell = chessBoard[cellIndex / 8][cellIndex % 8].pieceFEN;
	if (cell == '0')
		return 0;
	if (cell < 'Z' && pickUpPiece.pieceFEN > 'a' || cell > 'a' && pickUpPiece.pieceFEN < 'Z')
		return 1;
	return 2;
}

bool ChessEngineManager::capturePromote(int& cellIndex) {
	if (cellOccupied(cellIndex) == 2)
		return false;
	// black pawn en passant white pawn
	if (pickUpPiece.pieceFEN == 'p' && (pickUpIndex > 31 && pickUpIndex < 40) && chessBoard[cellIndex / 8][cellIndex % 8].pieceFEN == '0') {
		if (chessBoard[cellIndex / 8 - 1][cellIndex % 8].pieceFEN == 'P' && (cellIndex == pickUpIndex + 9 || cellIndex == pickUpIndex + 7)) {
			if (chessBoard[cellIndex / 8 - 1][cellIndex % 8].lastMoveNum == currentMoveNum - 1 && chessBoard[cellIndex / 8 - 1][cellIndex % 8].moveCount == 1) {
				chessBoard[cellIndex / 8 - 1][cellIndex % 8].pieceFEN = '0';
				return false;
			}
			else {
				cellIndex = pickUpIndex;
				return true;
			}
		}
	} // white pawn en passant black pawn
	else 
	if (pickUpPiece.pieceFEN == 'P' && (pickUpIndex > 23 && pickUpIndex < 32) && chessBoard[cellIndex / 8][cellIndex % 8].pieceFEN == '0') {
		if (chessBoard[cellIndex / 8 + 1][cellIndex % 8].pieceFEN == 'p' && (cellIndex == pickUpIndex - 9 || cellIndex == pickUpIndex - 7)) {
			if (chessBoard[cellIndex / 8 + 1][cellIndex % 8].lastMoveNum == currentMoveNum - 1 && chessBoard[cellIndex / 8 + 1][cellIndex % 8].moveCount == 1) {
				chessBoard[cellIndex / 8 + 1][cellIndex % 8].pieceFEN = '0';
				return false;
			}
			else {
				cellIndex = pickUpIndex;
				return true;
			}
		}
	}

	// pawn promotion
	if (pickUpPiece.pieceFEN == 'p' && cellIndex > 55 || pickUpPiece.pieceFEN == 'P' && cellIndex < 8) {
		isPromotion = true;
		promotionCell = cellIndex;

		int boardDimension = (windowWidth > windowHeight) ? windowHeight : windowWidth; // min{windowWidth, windowHeight}
		int gridCellDimension = boardDimension / 8;
		int w = gridCellDimension * 5;
		int h = gridCellDimension * 1.7;
		int x = std::max((promotionCell % 8) * gridCellDimension - w / 2, 0);
		x = std::min(x, windowWidth - w);
		int y = std::min((promotionCell / 8) * gridCellDimension, windowHeight - h);
		promotionBack = { x, y, w, h };
		promotionChoices = { x + w / 6, y + h - (3 * h / 4), w - w / 3, h / 2 };

		chessBoard[promotionCell / 8][promotionCell % 8] = pickUpPiece;
		chessBoard[promotionCell / 8][promotionCell % 8].lastMoveNum = currentMoveNum;
		chessBoard[promotionCell / 8][promotionCell % 8].moveCount++;
		return true;
	}

	// Castling
	if ((pickUpPiece.pieceFEN == 'k' || pickUpPiece.pieceFEN == 'K') && pickUpPiece.lastMoveNum == 0) {
		ChessEngineManagerUtil::Piece rookR = chessBoard[pickUpIndex / 8][(pickUpIndex + 3) % 8];
		ChessEngineManagerUtil::Piece rookL = chessBoard[pickUpIndex / 8][(pickUpIndex - 4) % 8];
		if (cellIndex == pickUpIndex + 2 && (rookR.lastMoveNum == 0 && (rookR.pieceFEN == 'r' || rookR.pieceFEN == 'R'))) {
			if (!xPathBlocked(pickUpIndex, pickUpIndex + 3)) {
				chessBoard[cellIndex / 8][cellIndex % 8] = pickUpPiece;
				chessBoard[cellIndex / 8][cellIndex % 8].lastMoveNum = currentMoveNum;
				chessBoard[cellIndex / 8][cellIndex % 8].moveCount++;
				rookR.lastMoveNum = currentMoveNum;
				rookR.moveCount++;
				chessBoard[cellIndex / 8][(cellIndex - 1) % 8] = rookR;
				chessBoard[pickUpIndex / 8][(pickUpIndex + 3) % 8].pieceFEN = '0';
				return true;
			}
			else {
				 // to cancle the move
				cellIndex = pickUpIndex;
				return true;
			}

		}
		else if (cellIndex == pickUpIndex - 2 && (rookL.lastMoveNum == 0 && (rookL.pieceFEN == 'r' || rookL.pieceFEN == 'R'))) {
			if (!xPathBlocked(pickUpIndex, pickUpIndex - 4)) {
				chessBoard[cellIndex / 8][cellIndex % 8] = pickUpPiece;
				chessBoard[cellIndex / 8][cellIndex % 8].lastMoveNum = currentMoveNum;
				chessBoard[cellIndex / 8][cellIndex % 8].moveCount++;
				rookL.lastMoveNum = currentMoveNum;
				rookL.moveCount++;
				chessBoard[cellIndex / 8][(cellIndex + 1) % 8] = rookL;
				chessBoard[pickUpIndex / 8][(pickUpIndex - 4) % 8].pieceFEN = '0';
				return true;
			}
			else {
				// to cancle the move
				cellIndex = pickUpIndex;
				return true;
			}
		}
	}

	return false;
}

void ChessEngineManager::promotePawn() {
	int x, y;
	SDL_GetMouseState(&x, &y);
	if ((x < promotionChoices.x || x > promotionChoices.x + promotionChoices.w) || (y < promotionChoices.y || y > promotionChoices.y + promotionChoices.h))
		return;

	switch ((x - promotionChoices.x) / (promotionChoices.w / 4)) {
	case 0:
		chessBoard[promotionCell / 8][promotionCell % 8].pieceFEN = (promotionCell < 8) ? 'Q' : 'q';
		break;
	case 1:
		chessBoard[promotionCell / 8][promotionCell % 8].pieceFEN = (promotionCell < 8) ? 'B' : 'b';
		break;
	case 2:
		chessBoard[promotionCell / 8][promotionCell % 8].pieceFEN = (promotionCell < 8) ? 'N' : 'n';
		break;
	case 3:
		chessBoard[promotionCell / 8][promotionCell % 8].pieceFEN = (promotionCell < 8) ? 'R' : 'r';
		break;
	}
	isPromotion = false;
	positionFEN = boardToFEN(chessBoard);

	isTurn = !isTurn;

	char move[6] = { '\0' };
	move[0] = (pickUpIndex % 8 + 'a');
	move[1] = (7 - pickUpIndex / 8 + '1');
	move[2] = (putDownIndex % 8 + 'a');
	move[3] = (7 - putDownIndex / 8 + '1');
	move[4] = chessBoard[promotionCell / 8][promotionCell % 8].pieceFEN;
	onMyMove(positionFEN);
}

void ChessEngineManager::autoQueen() {
	chessBoard[promotionCell / 8][promotionCell % 8].pieceFEN = (promotionCell < 8) ? 'Q' : 'q';
	isPromotion = false;
	positionFEN = boardToFEN(chessBoard);

	isTurn = !isTurn;
	onMyMove(positionFEN);
}

bool ChessEngineManager::xPathBlocked(int index1, int index2) {
	// check indices on same y level
	bool blocked = false;
	if (index1 > index2) {
		int temp = index1;
		index1 = index2;
		index2 = temp;
	}

	for (int i = index1 + 1; i < index2; i++) {
		if (chessBoard[i / 8][i % 8].pieceFEN != '0')
			return true;
	}

	return blocked;
}

bool ChessEngineManager::yPathBlocked(int index1, int index2) {
	bool blocked = false;
	
	return blocked;
}

bool ChessEngineManager::DiagonalPathBlocked(int index1, int index2) {
	bool blocked = false;
	//for (int i = 0;)

	return blocked;
}