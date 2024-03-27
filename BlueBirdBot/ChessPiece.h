#pragma once

struct Move {
	int startIndex;
	int endIndex;
};

class ChessPiece {
public:
	void pieceInit(char);
	bool active = true; // false if captured
	char type; // char representing type
	char pinnedDirectionFlags = 0; // each bit represents a direction the piece is pinned from
	int value; // piece value
	Move movementTypes[8]; // types of movement the piece can take
private:

};