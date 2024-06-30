#pragma once
#include <iostream>
#include <string>

struct Column {
	Column();
	Column(char, char, char, char, char, char, char, char);
	char row[8]; // char representing a piece
	char& operator[](int);
	char operator[](int) const;
};



class ChessBoard {
public:
	ChessBoard();
	void FENtoBoard(std::string);
	std::string getFEN();
	Column& operator[](int); // Use [x][y] to access board cell
	Column operator[](int) const; // Use [x][y] to access board cell
private:
	Column chessBoard[8];
};

std::ostream& operator<<(std::ostream&, const ChessBoard&);