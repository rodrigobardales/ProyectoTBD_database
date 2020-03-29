#pragma once
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <iostream>
#include <fstream>
#include <string>
#include "Datatype.h"

using namespace std;

class fileSystem
{
public:
	void createDatabase(char[30]);
	void createTable(char[30]);
	void createColumna(char[30], int, char[30], int, int);
	char* initBitMap(int);
	int getPosLibre(char*, int);
	char* setOf(char* bitmap, int num);
	char* setOn(char* bitMap, int nBlock);
	void printBitMap(char* bitMap, int);
	int getId(char[30]);
	void printTable(char[30]);
	void IngresarRegistro(char[30]);
	void writebitMap(char[30], char*);
	void selectall(char[30]);
	void dropRegistro(char[30]);
	int getPosLibreTYC(char[30], int);
	int decidePos(char[30], int);
	char retorntype(int);
	void updateRegistro(char[30]);
	bool findTable(char[30], char name[30]);
	void droptable(char[30]);
};

#endif // !FILESYSTEM_H
