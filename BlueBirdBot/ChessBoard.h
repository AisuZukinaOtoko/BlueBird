#pragma once
#include <string>

struct Column {
	Column();
	Column(char, char, char, char, char, char, char, char);
	char row[8]; // char representing a piece
	char& operator[](int);
};

class ChessBoard {
public:
	ChessBoard();
	void FENtoBoard(std::string);
	std::string getFEN();
	void printBoard();
	Column& operator[](int); // Use [x][y] to access board cell

private:
	Column chessBoard[8];
};