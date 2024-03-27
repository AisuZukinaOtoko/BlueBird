#include "ChessBoard.h"
#include "Assert.h"

Column::Column() {
	for (int i = 0; i < 8; i++) {
		row[i] = '#';
	}
}

Column::Column(char p1, char p2, char p3, char p4, char p5, char p6, char p7, char p8) {
	row[0] = p1;
	row[1] = p2;
	row[2] = p3;
	row[3] = p4;
	row[4] = p5;
	row[5] = p6;
	row[6] = p7;
	row[7] = p8;
}

char& Column::operator[](int index) {
	CORE_ASSERT(index >= 0 && index < 8, "Row index out of range.");
	return row[index];
}

ChessBoard::ChessBoard() {
	chessBoard[7] = {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'};
	chessBoard[6] = {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'};

	chessBoard[1] = {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'};
	chessBoard[0] = {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'};
}

Column& ChessBoard::operator[](int index) {
	CORE_ASSERT(index >= 0 && index < 8, "Column index out of range.");
	return chessBoard[index];
}

void ChessBoard::printBoard() {
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
			std::cout << chessBoard[x][y] << " ";
		}
		std::cout << "\n";
	}
}

void ChessBoard::FENtoBoard(std::string FEN) {
	int x = 0, y = 0;
	for (int i = 0; i < FEN.length(); i++) {
		if (FEN[i] == '/') {
			y++;
			x = 0;
			continue;
		}
		else if (FEN[i] > 48 && FEN[i] < 57) { // a number
			for (int k = 0; k < FEN[i] - '0'; k++) {
				chessBoard[x + k][y] = '#';
			}
			x += FEN[i] - '0';
			continue;
		}
		chessBoard[x][y] = FEN[i];
		x++;
	}
}

std::string ChessBoard::getFEN() {
	std::string FEN = "";
	for (int y = 0; y < 8; y++) {
		int gap = 0;
		for (int x = 0; x < 8; x++) {
			if (chessBoard[x][y] != '#' && gap) {
				FEN += std::to_string(gap) + chessBoard[x][y];
				gap = 0;
			}
			else if (chessBoard[x][y] != '#') {
				FEN += chessBoard[x][y];
			}
			else { // '#'
				gap++;
			}
		}
		if (gap)
			FEN += std::to_string(gap);
		if (y < 7)
			FEN += '/';
	}
	return FEN;
}