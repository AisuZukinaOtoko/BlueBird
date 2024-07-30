#include <iostream>
#include "IcePickEngineManager/ChessEngineManager.h"
#include "BlueBirdBot/BlueBird.h"

int main(int argc, char* argv[]) {	
	ChessEngineManager IcePickEngineManager;
	BlueBird TheLegend;
	IcePickEngineManager.engineInit(&TheLegend);
	IcePickEngineManager.engineRun();
	return 0;
}