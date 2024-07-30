#include "ChessPiece.h"
#include "Assert.h"

void ChessPiece::pieceInit(char pieceType) {
	Move allMovementTypes[16] = { 
		{1, 1}, {1, -1}, {-1, -1}, {-1, 1},
		{1, 0}, {-1, 0}, {0, 1}, {0, -1},
		{-1, 2}, {1, 2}, {2, 1}, {2, -1}, 
		{-2, 1}, {-2, -1}, {-2, 1}, {-2, -1} };

	switch (pieceType) {
	case 'p':
		movementTypes[0] = {0, -1};
		movementTypes[1] = {0, -2};
		movementTypes[2] = {-1, -1};
		movementTypes[3] = {1, -1};
		value = 100;
		break;
	case 'P':
		movementTypes[0] = {0, 1};
		movementTypes[1] = {0, 2};
		movementTypes[2] = {-1, 1};
		movementTypes[3] = {1, 1};
		value = 100;
		break;

	case 'b':
	case 'B':
		for (int i = 0; i < 4; i++)
			movementTypes[i] = allMovementTypes[i];
		value = 300;
		break;

	case 'n':
	case 'N':
		for (int i = 0; i < 8; i++)
			movementTypes[i] = allMovementTypes[i + 8];
		value = 300;
		break;

	case 'r':
	case 'R':
		for (int i = 0; i < 4; i++)
			movementTypes[i] = allMovementTypes[i + 4];
		value = 500;
		break;

	case 'q':
	case 'Q':
		for (int i = 0; i < 8; i++)
			movementTypes[i] = allMovementTypes[i];
		value = 900;
		break;

	case 'k':
	case 'K':
		for (int i = 0; i < 8; i++)
			movementTypes[i] = allMovementTypes[i];
		value = 1000000000;
		break;
	default:
		CORE_ASSERT(false, "Unknown piece type.");
		break;
		
	}

}