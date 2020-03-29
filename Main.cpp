#include "fileSystem.h"
#include <iostream>
#include <fstream>
#include <conio.h>
using namespace std;

int main()
{
	char file[30] = "1.bin";
	char file2[30] = "2.bin";

	fileSystem sys;

	sys.createDatabase(file);

	bool sys1 = true;
	char actual[30];
	strcpy_s(actual, file);
	do
	{
		char comand[30];

		cout << "Actual:" << actual << endl;
		cout << "Command:";
		cin >> comand;


		if (strcmp(comand, "create") == 0)
		{

			sys.createTable(actual);
		}
		if (strcmp(comand, "update") == 0)
		{

			sys.updateRegistro(actual);
		}

		if (strcmp(comand, "select") == 0)
		{

			sys.selectall(actual);
		}
		 
		if (strcmp(comand, "insert") == 0)
		{

			sys.IngresarRegistro(actual);
		}
		if (strcmp(comand, "dropregister") == 0)
		{

			sys.dropRegistro(actual);
		}

		if (strcmp(comand, "droptable") == 0)
		{

			sys.droptable(actual);
		}

		if (strcmp(comand, "salir") == 0)
		{
			sys1 = false;
		}

	} while (sys1 == true);

	return 0;
}