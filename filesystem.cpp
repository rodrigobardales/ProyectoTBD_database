#include "fileSystem.h"
#include <iostream>
#include <string>
#include <fstream>
#include "estructuras.h"

using namespace std;

void fileSystem::createDatabase(char file[30])
{

	ofstream archivo(file, ios::in | ios::app | ios::binary);

	if (!archivo)
	{
		cout << "error al abrir archivo" << endl;
	}
	else
	{
		cout << "Ingrese Nombre de Base de Datos" << endl;
		char name[30];
		cin >> name;

		cout << "Seleccion dato" << endl;
		cout << "1.Gb" << endl;
		cout << "2.Mb" << endl;
		int op = 0;
		cin >> op;

		cout << "Ingrese size de bloque 512 a 1024" << endl;
		int tam_block;
		do
		{
			cin >> tam_block;
		} while (tam_block < 512 || tam_block > 4096);
		tam_block -= 4 - sizeof(blockNormal);

		cout << "Ingrese tamano de base de datos" << endl;
		int tam2_size;
		cin >> tam2_size;

		auto size = 0;

		if (op == 1)
		{
			size = (((tam2_size * 1024) * 1024) * 1024);
		}
		else
		{
			size = ((tam2_size * 1024) * 1024);
		}
		int total_block = (size / tam_block);
		int totalTablas = (tam_block / sizeof(table));
		int totalColumnas = (tam_block / sizeof(Columna));
		int residuo_tablas = (sizeof(table) * totalTablas) - tam_block;
		int residuo_columna = (sizeof(Columna) * totalTablas) - tam_block;

		///INGRESAR A metadata
		metadata meta;

		strcpy_s(meta.name, name);
		meta.gb_mb = op;
		meta.sizebytes = size;
		meta.size_blocks = tam_block;
		meta.id_tablas = 0;
		meta.cant_registrosTabla = tam_block / sizeof(table);
		meta.cant_registrosColumna = tam_block / sizeof(Columna);
		meta.cant_blocks = total_block;
		meta.residuo_Columnas = tam_block - (meta.cant_registrosColumna * sizeof(Columna));
		meta.residuo_tablas = tam_block - (meta.cant_registrosTabla * sizeof(table));
		archivo.write(reinterpret_cast<const char*>(&meta), sizeof(metadata));

		///bitmapp

		bitmap bit;
		bit.bitmap = initBitMap(total_block);

		archivo.write(reinterpret_cast<const char*>(&bit), sizeof(bitmap));

		for (int i = 0; i < total_block; i++)
		{
			blockNormal as;
			archivo.write(reinterpret_cast<const char*>(&as), sizeof(blockNormal));
		}

		cout << "Archivo Generado exitosamente" << endl;
		archivo.close();
	}
}

void fileSystem::createTable(char file[30])
{

	fstream archivo(file, ios::in | ios::out | ios::binary);
	if (!archivo)
	{
		cout << "no se pudo abrir el archivo" << endl;
	}
	else
	{
		cout << "Ingrese Nombre de Tabla" << endl;
		char name[30];
		cin >> name;
		int posLibre = decidePos(file, 1);
		metadata infromacion_base;
		int pointer = sizeof(metadata) + sizeof(bitmap);
		int sumpointer = sizeof(blockNormal) * posLibre;
		int id = getId(file);
		archivo.seekg(0, ios::beg);
		archivo.read(reinterpret_cast<char*>(&infromacion_base), sizeof(metadata));

		archivo.seekg(pointer + sumpointer);

		tableBlock tmp;

		archivo.read(reinterpret_cast<char*>(&tmp), sizeof(tableBlock));

		int posActual = -1;
		if (tmp.length == 0)
		{
			tmp.data = new table[infromacion_base.cant_registrosTabla];
			tmp.data[0].id = id;
			strcpy_s(tmp.data[0].name, name);
			tmp.data[0].used = true;
			tmp.length += 1;
			posActual = 0;
			tmp.type = 'T';
		}
		else
		{

			if (tmp.length == infromacion_base.cant_registrosTabla) {}
			else
			{
				for (int i = 0; i < infromacion_base.cant_registrosTabla; i++)
				{
					if (tmp.data[i].used == false)
					{
						tmp.data[i].id = id;
						strcpy_s(tmp.data[i].name, name);
						tmp.data[i].used = true;
						tmp.length += 1;
						posActual = i;
						break;
					}
				}
			}
		}
		int SizeTotalColumnas = 0;
		cout << "Ingrese cantidad de Columna a INGRESAR" << endl;
		int ColumnasNum;
		int contInt = 0;
		int contDouble = 0;
		int contvarchar = 0;

		cin >> ColumnasNum;
		for (int i = 0; i < ColumnasNum; i++)
		{
			cout << "Ingrese Nombre Columna" << endl;
			char name[30];
			cin >> name;
			cout << "Seleccione tipo de dato" << endl;
			cout << "1.Int" << endl;
			cout << "2.Double" << endl;
			cout << "3.Varchar" << endl;

			int op;
			cin >> op;
			int size_Var = 0;
			if (op == 3)
			{
				cout << "Ingrese Tam Varchar" << endl;
				do
				{
					cin >> size_Var;
				} while (size_Var > 2000);
				SizeTotalColumnas += size_Var;
				contvarchar++;
			}
			if (op == 1)
			{
				SizeTotalColumnas += 4;
				contInt++;
			}
			if (op == 2)
			{
				SizeTotalColumnas += 8;
				contDouble++;
			}
			createColumna(file, id, name, op, size_Var);
		}
		tmp.data[posActual].cant_Columnas = ColumnasNum;
		tmp.data[posActual].cant_Int = contInt;
		tmp.data[posActual].cant_Double = contDouble;
		tmp.data[posActual].cant_varchar = contvarchar;

		tmp.data[posActual].cant_Registro = infromacion_base.size_blocks / (SizeTotalColumnas);
		tmp.type = 'T';

		blockNormal tmp2;
		archivo.seekp(pointer + sumpointer);
		archivo.write(reinterpret_cast<const char*>(&tmp), sizeof(tableBlock));
		archivo.close();
	}
}

void fileSystem::createColumna(char file[30], int id, char name[30], int tipo, int size_if_varchar)
{
	fstream archivo(file, ios::in | ios::out | ios::binary);
	if (!archivo)
	{
		cout << "no se pudo abrir el archivo" << endl;
	}
	else
	{
		int posLibre = decidePos(file, 2);
		metadata infromacion_base;
		int pointer = sizeof(metadata) + sizeof(bitmap);
		int sumpointer = sizeof(blockNormal) * posLibre;
		archivo.seekg(0, ios::beg);
		archivo.read(reinterpret_cast<char*>(&infromacion_base), sizeof(metadata));
		archivo.seekg(pointer + sumpointer);

		ColumnBlock tmp;

		archivo.read(reinterpret_cast<char*>(&tmp), sizeof(ColumnBlock));

		//si no hay tablas
		if (tmp.length == 0)
		{
			tmp.data = new Columna[infromacion_base.cant_registrosTabla];
			tmp.data[0].id = id;
			strcpy_s(tmp.data[0].name, name);
			tmp.data[0].used = true;
			tmp.length += 1;
			tmp.data[0].size_if_varchar = size_if_varchar;
			tmp.data[0].tipo = tipo;
			tmp.type = 'C';
		}
		else
		{
			//si si hay tabla
			if (tmp.length == infromacion_base.cant_registrosColumna) {}
			else
			{
				for (int i = 0; i < infromacion_base.cant_registrosColumna; i++)
				{
					if (tmp.data[i].used == false)
					{
						tmp.data[i].id = id;
						strcpy_s(tmp.data[i].name, name);
						tmp.data[i].used = true;
						tmp.length += 1;
						tmp.data[i].size_if_varchar = size_if_varchar;
						tmp.data[i].tipo = tipo;
						tmp.type = 'C';
						break;
					}
				}
			}
		}
		tmp.type = 'C';
		archivo.seekp(pointer + sumpointer);
		archivo.write(reinterpret_cast<const char*>(&tmp), sizeof(ColumnBlock));
		archivo.close();
	}
}

void fileSystem::IngresarRegistro(char file[30])
{
	fstream archivo(file, ios::in | ios::out | ios::binary);
	if (!archivo)
	{
		cout << "error al abrir el archivo" << endl;
	}
	else
	{
		cout << "Ingrese Nombre de tabla a insertar" << endl;
		char name[30];
		cin >> name;
		if (findTable(file, name))
		{
			int posLibre = decidePos(file, 3);
			metadata infromacion_base;
			bitmap bit;
			int pointer = sizeof(metadata) + sizeof(bitmap);
			int sumpointer = sizeof(blockNormal) * posLibre;
			///Vector para saber posiciones llenas
			vector<int> posUsadas;
			///////

			/////variables de tabla y columna
			ColumnBlock columnas;
			int posDataColumna;
			registerBlock registroI;
			tableBlock tabla;
			int posDataTabla;
			blockNormal tmp;
			tableBlock tablaUsando;

			archivo.seekg(0, ios::beg);
			archivo.read(reinterpret_cast<char*>(&infromacion_base), sizeof(metadata));
			archivo.read(reinterpret_cast<char*>(&bit), sizeof(bitmap));

			const int SHIFT = 8 * sizeof(char) - 1;
			const char MASK = 1 << SHIFT;

			///saber posiciones ocupadas
			for (int i = 0; i < infromacion_base.cant_blocks / 8; i++)
			{
				char value;
				value = bit.bitmap[i];
				for (int c = 1; c <= SHIFT + 1; c++)
				{
					char z = -1;
					(value & MASK ? z = '1' : z = '0');
					if (z == '1')
					{

						posUsadas.push_back({ (8 * i) + (c - 1) });
					}
					value <<= 1;
				}
			}
			///saber posiciones ocupadas


			//encontrar pos de tabla a insertar
			for (int v : posUsadas)
			{
				archivo.seekg(pointer + (sizeof(blockNormal) * v));
				archivo.read(reinterpret_cast<char*>(&tabla), sizeof(tableBlock));
				//cout <<"tippe"<<tabla.type << endl;
				if (tabla.type == 'T')
				{
					for (int i = 0; i < infromacion_base.cant_registrosTabla; i++)
					{
						if (tabla.data[i].used == true)
						{
							if (strcmp(tabla.data[i].name, name) == 0)
							{
								tabla.data[i].cant_Columnas;
								posDataTabla = i;
								tablaUsando = tabla;
								posDataColumna = v;
								break;
							}
						}
					}
				}
			}

			Columna* columnasUsando = new Columna[tablaUsando.data[posDataTabla].cant_Columnas];
			//encontrar pos de columna a insertar
			int cont = 0;
			for (int v : posUsadas)
			{
				archivo.seekg(pointer + (sizeof(ColumnBlock) * v));
				archivo.read(reinterpret_cast<char*>(&columnas), sizeof(ColumnBlock));
				if (columnas.type == 'C')
				{
					for (int j = 0; j < infromacion_base.cant_registrosColumna; j++)
					{
						if (columnas.data[j].used == true)
						{
							if (columnas.data[j].id == tablaUsando.data[posDataTabla].id)
							{

								columnasUsando[cont] = columnas.data[j];
								cont++;
							}
						}
					}
				}
			}

			archivo.seekg(pointer + sumpointer);
			archivo.read(reinterpret_cast<char*>(&registroI), sizeof(registerBlock));
			if (registroI.length == 0)
			{
				registroI.data = new registro[tablaUsando.data[posDataTabla].cant_Registro];
				for (int i = 0; i < tablaUsando.data[posDataTabla].cant_Registro; i++)
				{
					registroI.data[i].id = tablaUsando.data[posDataTabla].id;
					registroI.data[i].dobles = new doble[tablaUsando.data[posDataTabla].cant_Double];
					registroI.data[i].enteros = new entero[tablaUsando.data[posDataTabla].cant_Int];
					registroI.data[i].var = new varchar[tablaUsando.data[posDataTabla].cant_varchar];
				}

				int tmpInt = 0;
				int tmpDouble = 0;
				int tmpChar = 0;
				char tipo[50];
				for (int j = 0; j < tablaUsando.data[posDataTabla].cant_Columnas; j++)
				{
					if (columnasUsando[j].tipo == 1)
					{
						strcpy_s(tipo, "Int");
					}
					if (columnasUsando[j].tipo == 2)
					{
						strcpy_s(tipo, "Double");
					}
					if (columnasUsando[j].tipo == 3)
					{
						strcpy_s(tipo, "Varchar");
					}
					cout << "Ingrese valor para columna: " << columnasUsando[j].name << " de tipo: " << tipo << endl;
					;
					int entradaInt;
					double entradaDouble = NULL;
					char* entravar = new char[200];

					//int
					if (columnasUsando[j].tipo == 1)
					{
						cin >> entradaInt;

						strcpy_s(registroI.data[0].enteros[tmpInt].name, columnasUsando[j].name);
						registroI.data[0].enteros[tmpInt].dato = entradaInt;
						cout << "numero:" << registroI.data[0].enteros[tmpInt].dato << endl;
						tmpInt++;
					}
					if (columnasUsando[j].tipo == 2)
					{
						cin >> entradaDouble;

						strcpy_s(registroI.data[0].dobles[tmpDouble].name, columnasUsando[j].name);
						registroI.data[0].dobles[tmpDouble].dato = entradaDouble;
						tmpDouble++;
					}
					//varcher
					if (columnasUsando[j].tipo == 3)
					{
						do
						{
							cout << "Ingrese longitud de varchar" << endl;
							cin >> entravar;
						} while (strlen(entravar) > columnasUsando[j].size_if_varchar);

						strcpy_s(registroI.data[0].var[tmpChar].name, columnasUsando[j].name);
						registroI.data[0].var[tmpChar].dato = entravar;

						tmpChar++;
					}
					registroI.data[0].used = true;
				}
				registroI.length += 1;
			}
			else
			{
				if (registroI.length == infromacion_base.cant_registrosColumna)
				{
				}
				else
				{
					for (int i = 0; i < tablaUsando.data[posDataTabla].cant_Registro; i++)
					{
						if (registroI.data[i].used == false)
						{
							int tmpInt = 0;
							int tmpDouble = 0;
							int tmpChar = 0;
							char tipo[50];
							for (int j = 0; j < tablaUsando.data[posDataTabla].cant_Columnas; j++)
							{
								if (columnasUsando[j].tipo == 1)
								{
									strcpy_s(tipo, "Int");
								}
								if (columnasUsando[j].tipo == 2)
								{
									strcpy_s(tipo, "Double");
								}
								if (columnasUsando[j].tipo == 3)
								{
									strcpy_s(tipo, "Varchar");
								}
								cout << "Ingrese valor para columna: " << columnasUsando[j].name << " de tipo: " << tipo << endl;
								;
								int entradaInt;
								double entradaDouble = NULL;
								char* entravar = new char[200];

								if (columnasUsando[j].tipo == 1)
								{
									cin >> entradaInt;

									strcpy_s(registroI.data[i].enteros[tmpInt].name, columnasUsando[j].name);
									registroI.data[i].enteros[tmpInt].dato = entradaInt;
									cout << "numero:" << registroI.data[0].enteros[tmpInt].dato << endl;
									tmpInt++;
								}
								if (columnasUsando[j].tipo == 2)
								{
									cin >> entradaDouble;

									strcpy_s(registroI.data[i].dobles[tmpDouble].name, columnasUsando[j].name);
									registroI.data[i].dobles[tmpDouble].dato = entradaDouble;
									tmpDouble++;
								}
								if (columnasUsando[j].tipo == 3)
								{
									do
									{
										cout << "Ingrese valor de varchar de longitud correcta" << endl;
										cin >> entravar;
									} while (strlen(entravar) > columnasUsando[j].size_if_varchar);

									strcpy_s(registroI.data[i].var[tmpChar].name, columnasUsando[j].name);
									registroI.data[i].var[tmpChar].dato = entravar;
									tmpChar++;
								}
								registroI.data[i].used = true;
							}
							registroI.length += 1;
							break;
						}
					}
				}
			}
			registroI.type = 'R';
			archivo.seekp(pointer + sumpointer);
			archivo.write(reinterpret_cast<const char*>(&registroI), sizeof(blockNormal));
		}
		else
		{
			cout << "no existe la tabla" << endl;
		}
	}
}

void fileSystem::selectall(char file[30])
{
	fstream archivo(file, ios::in | ios::out | ios::binary);
	if (!archivo)
	{
		cout << "error al abrir el archivo" << endl;
	}
	else
	{
		cout << "Ingrese Nombre de tabla " << endl;
		char name[30];
		cin >> name;

		if (findTable(file, name))
		{
			metadata infromacion_base;
			bitmap bit;
			int pointer = sizeof(metadata) + sizeof(bitmap);

			///Vector para saber posiciones llenas
			vector<int> posUsadas;
			vector<int> posRegistros;
			///////

			/////variables de tabla y columna
			ColumnBlock columnas;
			int posDataColumna;
			registerBlock registroI;
			tableBlock tabla;
			int posDataTabla;
			blockNormal tmp;
			tableBlock tablaUsando;

			archivo.seekg(0, ios::beg);
			archivo.read(reinterpret_cast<char*>(&infromacion_base), sizeof(metadata));
			archivo.read(reinterpret_cast<char*>(&bit), sizeof(bitmap));

			const int SHIFT = 8 * sizeof(char) - 1;
			const char MASK = 1 << SHIFT;

			///saber posiciones ocupadas
			for (int i = 0; i < infromacion_base.cant_blocks / 8; i++)
			{
				char value;
				value = bit.bitmap[i];
				for (int c = 1; c <= SHIFT + 1; c++)
				{
					char z = -1;
					(value & MASK ? z = '1' : z = '0');
					if (z == '1')
					{

						posUsadas.push_back({ (8 * i) + (c - 1) });
					}
					value <<= 1;
				}
			}

			//encontrar pos de tabla
			for (int v : posUsadas)
			{
				archivo.seekg(pointer + (sizeof(blockNormal) * v));
				archivo.read(reinterpret_cast<char*>(&tabla), sizeof(tableBlock));

				if (tabla.type == 'T')
				{
					for (int i = 0; i < infromacion_base.cant_registrosTabla; i++)
					{
						if (tabla.data[i].used == true)
						{
							if (strcmp(tabla.data[i].name, name) == 0)
							{
								cout << "nombre:" << tabla.data[i].cant_Columnas << endl;
								//cout << "entro aqui" << endl;
								tabla.data[i].cant_Columnas;
								posDataTabla = i;
								tablaUsando = tabla;
								posDataColumna = v;
								break;
							}
						}
					}
				}
			}
			Columna* columnasUsando = new Columna[tablaUsando.data[posDataTabla].cant_Columnas];
			//encontrar pos de columna a insertar
			int cont = 0;
			for (int v : posUsadas)
			{
				archivo.seekg(pointer + (sizeof(blockNormal) * v));
				archivo.read(reinterpret_cast<char*>(&columnas), sizeof(ColumnBlock));
				if (columnas.type == 'C')
				{
					for (int i = 0; i < infromacion_base.cant_registrosColumna; i++)
					{
						if (columnas.data[i].used == true)
						{
							if (columnas.data[i].id == 1)
							{

								columnasUsando[cont] = columnas.data[i];
								cont++;
							}
						}
					}
				}
			}
			///encontras pos columnas
			for (int v : posUsadas)
			{
				registerBlock tmp;
				archivo.seekg(pointer + (sizeof(blockNormal) * v));
				archivo.read(reinterpret_cast<char*>(&tmp), sizeof(ColumnBlock));
				if (tmp.type == 'R')
				{
					if (tmp.data[0].id == tablaUsando.data[posDataTabla].id)
					{
						posRegistros.push_back({ v });
					}
				}
			}

			for (int i = 0; i < posRegistros.size(); i++)
			{
				archivo.seekg(pointer + (sizeof(registerBlock) * posRegistros[i]));
				archivo.read(reinterpret_cast<char*>(&registroI), sizeof(registerBlock));
				for (int p = 0; p < tablaUsando.data[posDataTabla].cant_Registro; p++)
				{
					if (registroI.data[p].used == true)
					{
						int tmpInt = 0;
						int tmpDouble = 0;
						int tmpChar = 0;
						for (int j = 0; j < tablaUsando.data[posDataTabla].cant_Columnas; j++)
						{
							if (columnasUsando[j].tipo == 1)
							{
								cout << "  columna: " << columnasUsando[j].name << " Valor: " << registroI.data[p].enteros[tmpInt].dato;
								tmpInt++;
							}
							if (columnasUsando[j].tipo == 2)
							{
								cout << "  columna: " << columnasUsando[j].name << " Valor: " << registroI.data[p].dobles[tmpDouble].dato;
								tmpDouble++;
							}
							if (columnasUsando[j].tipo == 3)
							{
								cout << "  columna: " << columnasUsando[j].name << " Valor: " << registroI.data[p].var[tmpChar].dato;
								tmpChar++;
							}
						}
						cout << "" << endl;
					}
				}
			}
		}
		else
		{
			cout << "no se encuentra la tabla" << endl;
		}
	}
}

void fileSystem::droptable(char file[30])
{
	fstream archivo(file, ios::in | ios::out | ios::binary);
	if (!archivo)
	{
		cout << "error al abrir el archivo" << endl;
	}
	else
	{
		cout << "Ingrese Nombre de tabla " << endl;
		char name[30];
		cin >> name;
		if (findTable(file, name))
		{
			metadata infromacion_base;
			bitmap bit;
			int pointer = sizeof(metadata) + sizeof(bitmap);

			///Vector para saber posiciones llenas
			vector<int> posUsadas;
			vector<int> posRegistros;
			///////

			/////variables de tabla y columna
			ColumnBlock columnas;
			int posDataColumna;
			registerBlock registroI;
			tableBlock tabla;
			int posDataTabla;
			blockNormal tmp;
			tableBlock tablaUsando;
			ColumnBlock bloqueaGuardar;
			int posBloqueTabla;
			int posBloqueColumna;

			archivo.seekg(0, ios::beg);
			archivo.read(reinterpret_cast<char*>(&infromacion_base), sizeof(metadata));
			archivo.read(reinterpret_cast<char*>(&bit), sizeof(bitmap));

			const int SHIFT = 8 * sizeof(char) - 1;
			const char MASK = 1 << SHIFT;

			///saber posiciones ocupadas
			for (int i = 0; i < infromacion_base.cant_blocks / 8; i++)
			{
				char value;
				value = bit.bitmap[i];
				for (int c = 1; c <= SHIFT + 1; c++)
				{
					char z = -1;
					(value & MASK ? z = '1' : z = '0');
					if (z == '1')
					{
						posUsadas.push_back({ (8 * i) + (c - 1) });
					}
					value <<= 1;
				}
			}
			//encontrar pos de tabla a insertar
			for (int v : posUsadas)
			{
				archivo.seekg(pointer + (sizeof(blockNormal) * v));
				archivo.read(reinterpret_cast<char*>(&tabla), sizeof(tableBlock));
				if (tabla.type == 'T')
				{
					for (int i = 0; i < infromacion_base.cant_registrosTabla; i++)
					{
						if (tabla.data[i].used == true)
						{
							if (strcmp(tabla.data[i].name, name) == 0)
							{
								tabla.data[i].cant_Columnas;
								posDataTabla = i;
								tablaUsando = tabla;
								posDataColumna = v;
								posBloqueTabla = v;
								break;
							}
						}
					}
				}
			}
			Columna* columnasUsando = new Columna[tablaUsando.data[posDataTabla].cant_Columnas];
			//encontrar pos de columna a insertar
			int cont = 0;
			for (int v : posUsadas)
			{
				archivo.seekg(pointer + (sizeof(blockNormal) * v));
				archivo.read(reinterpret_cast<char*>(&columnas), sizeof(ColumnBlock));
				if (columnas.type == 'C')
				{
					for (int i = 0; i < infromacion_base.cant_registrosColumna; i++)
					{
						if (columnas.data[i].used == true)
						{
							if (columnas.data[i].id == tablaUsando.data[posDataTabla].id)
							{
								columnas.data[i].used = false;

								columnasUsando[cont] = columnas.data[i];
								cont++;
								posBloqueColumna = v;
							}
						}
					}
				}
			}
			///encontras pos columnas

			for (int v : posUsadas)
			{
				registerBlock tmp;
				archivo.seekg(pointer + (sizeof(blockNormal) * v));
				archivo.read(reinterpret_cast<char*>(&tmp), sizeof(ColumnBlock));
				if (tmp.type == 'R')
				{
					if (tmp.data[0].id == tablaUsando.data[posDataTabla].id)
					{
						posRegistros.push_back({ v });
					}
				}
			}

			tablaUsando.data[posDataTabla].used = false;

			for (int i = 0; i < posRegistros.size(); i++)
			{
				archivo.seekg(pointer + (sizeof(registerBlock) * posRegistros[i]));
				archivo.read(reinterpret_cast<char*>(&registroI), sizeof(registerBlock));
				registroI.data[0].used = false;
				archivo.seekg(pointer + (sizeof(registerBlock) * posRegistros[i]));
				archivo.write(reinterpret_cast<const char*>(&registroI), sizeof(registerBlock));
			}

			archivo.seekp(pointer + (sizeof(registerBlock) * posBloqueTabla));
			archivo.write(reinterpret_cast<const char*>(&tablaUsando), sizeof(tableBlock));
			archivo.seekp(pointer + (sizeof(ColumnBlock) * posBloqueColumna));
			archivo.read(reinterpret_cast<char*>(&bloqueaGuardar), sizeof(tableBlock));
			for (int i = 0; i < infromacion_base.cant_registrosColumna; i++)
			{
				if (bloqueaGuardar.data[i].used == true)
				{
					if (bloqueaGuardar.data[i].id == tablaUsando.data[posDataTabla].id)
					{
						bloqueaGuardar.data[i].used;
					}
				}
			}

			archivo.seekp(pointer + (sizeof(ColumnBlock) * posBloqueColumna));
			archivo.write(reinterpret_cast<const char*>(&bloqueaGuardar), sizeof(blockNormal));
		}
		else
		{
			cout << "no se encuentra la tabla" << endl;
		}
	}
}

void fileSystem::dropRegistro(char file[30])
{
	fstream archivo(file, ios::in | ios::out | ios::binary);
	if (!archivo)
	{
		cout << "error al abrir el archivo" << endl;
	}
	else
	{
		cout << "Ingrese Nombre de tabla " << endl;
		char name[30];
		cin >> name;
		int posLibre = 2;
		if (findTable(file, name) == true)
		{
			metadata infromacion_base;
			bitmap bit;
			int pointer = sizeof(metadata) + sizeof(bitmap);
			int sumpointer = sizeof(blockNormal) * posLibre;
			///Vector para saber posiciones llenas
			vector<int> posUsadas;
			vector<int> posRegistros;
			///////

			/////variables de tabla y columna
			ColumnBlock columnas;
			int posDataColumna;
			registerBlock registroI;
			tableBlock tabla;
			int posDataTabla;
			blockNormal tmp;
			tableBlock tablaUsando;

			archivo.seekg(0, ios::beg);
			archivo.read(reinterpret_cast<char*>(&infromacion_base), sizeof(metadata));
			archivo.read(reinterpret_cast<char*>(&bit), sizeof(bitmap));

			const int SHIFT = 8 * sizeof(char) - 1;
			const char MASK = 1 << SHIFT;

			///saber posiciones ocupadas
			for (int i = 0; i < infromacion_base.cant_blocks / 8; i++)
			{
				char value;
				value = bit.bitmap[i];
				for (int c = 1; c <= SHIFT + 1; c++)
				{
					char z = -1;
					(value & MASK ? z = '1' : z = '0');
					if (z == '1')
					{

						posUsadas.push_back({ (8 * i) + (c - 1) });
					}
					value <<= 1;
				}
			}
			///saber posiciones ocupadas

			//encontrar pos de tabla a insertar
			for (int v : posUsadas)
			{
				archivo.seekg(pointer + (sizeof(blockNormal) * v));
				archivo.read(reinterpret_cast<char*>(&tabla), sizeof(tableBlock));
				if (tabla.type == 'T')
				{
					for (int i = 0; i < infromacion_base.cant_registrosTabla; i++)
					{
						if (tabla.data[i].used == true)
						{
							if (strcmp(tabla.data[i].name, name) == 0)
							{
								cout << "nombre:" << tabla.data[i].cant_Columnas << endl;
								tabla.data[i].cant_Columnas;
								posDataTabla = i;
								tablaUsando = tabla;
								posDataColumna = v;
								break;
							}
						}
					}
				}
			}
			Columna* columnasUsando = new Columna[tablaUsando.data[posDataTabla].cant_Columnas];
			//encontrar pos de columna a insertar
			int cont = 0;
			for (int v : posUsadas)
			{
				archivo.seekg(pointer + (sizeof(blockNormal) * v));
				archivo.read(reinterpret_cast<char*>(&columnas), sizeof(ColumnBlock));
				if (columnas.type == 'C')
				{
					for (int i = 0; i < infromacion_base.cant_registrosColumna; i++)
					{
						if (columnas.data[i].used == true)
						{
							if (columnas.data[i].id == 1)
							{
								columnasUsando[cont] = columnas.data[i];
								cont++;
							}
						}
					}
				}
			}
			///encontras pos columnas
			for (int v : posUsadas)
			{
				registerBlock tmp;
				archivo.seekg(pointer + (sizeof(blockNormal) * v));
				archivo.read(reinterpret_cast<char*>(&tmp), sizeof(ColumnBlock));
				if (tmp.type == 'R')
				{
					if (tmp.data[0].id == tablaUsando.data[posDataTabla].id)
					{
						posRegistros.push_back({ v });
					}
				}
			}

			int erase = NULL;
			double erase1 = NULL;
			char* erase2 = NULL;
			int type = NULL;
			bool found = false;
			int place = -1;
			registerBlock registro_guardar;

			for (int j = 0; j < 1; j++)
			{

				cout << "Ingrese valor de columna a borrar: " << columnasUsando[j].name << endl;
				if (columnasUsando[j].tipo == 1)
				{
					cin >> erase;
					type = columnasUsando[j].tipo;
				}
				if (columnasUsando[j].tipo == 2)
				{
					cin >> erase1;
					type = columnasUsando[j].tipo;
				}
				if (columnasUsando[j].tipo == 3)
				{
					do
					{
						cout << "Ingrese cantidad correcta de bytes" << endl;
						cin >> erase2;
					} while (strlen(erase2) > columnasUsando[j].size_if_varchar);
					type = columnasUsando[j].tipo;
				}
			}

			for (int i = 0; i < posRegistros.size(); i++)
			{
				archivo.seekg(pointer + (sizeof(registerBlock) * posRegistros[i]));
				archivo.read(reinterpret_cast<char*>(&registroI), sizeof(registerBlock));
				for (int k = 0; k < tablaUsando.data[posDataTabla].cant_Registro; k++)
				{
					if (registroI.data[k].used == true)
					{
						int tmpInt = 0;
						int tmpDouble = 0;
						int tmpChar = 0;
						char tipo[50];

						for (int p = 0; p < tablaUsando.data[posDataTabla].cant_Registro; p++)
						{
							if (registroI.data[p].used == true)
							{
								for (int j = 0; j < 1; j++)
								{
									if (type == 1)
									{
										if (registroI.data[p].enteros[j].dato == erase)
										{
											registroI.data[p].used = false;
											found = true;
											place = posRegistros[i];
											registro_guardar = registroI;
										}
									}
									if (type == 2)
									{
										if (registroI.data[p].dobles[j].dato == erase1)
										{
											registroI.data[p].used = false;
											found = true;
											place = posRegistros[i];
											registro_guardar = registroI;
										}
									}
									if (type == 3)
									{
										if (strcmp(registroI.data[p].var[j].dato, erase2) == 0)
										{
											registroI.data[p].used = false;
											found = true;
											place = posRegistros[i];
											registro_guardar = registroI;
										}
									}
								}
								tmp.length = tmp.length - 1;
								cout << "" << endl;
								break;
							}
						}
					}
					if (found == true)
					{
						break;
					}
				}
			}
			if (place != -1)
			{
				archivo.seekp(pointer + (sizeof(registerBlock) * place));
				archivo.write(reinterpret_cast<const char*>(&registro_guardar), sizeof(registerBlock));
			}
			else
			{
				cout << "error con la data" << endl;
			}
		}
		else
		{
			cout << "no se encontro la tabla" << endl;
		}
	}

}

void fileSystem::updateRegistro(char file[30])
{
	fstream archivo(file, ios::in | ios::out | ios::binary);
	if (!archivo)
	{
		cout << "error al abrir el archivo" << endl;
	}
	else
	{

		cout << "Ingrese Nombre de tabla " << endl;
		char name[30];
		cin >> name;
		int posLibre = 2;

		metadata infromacion_base;
		bitmap bit;
		int pointer = sizeof(metadata) + sizeof(bitmap);
		int sumpointer = sizeof(blockNormal) * posLibre;
		///Vector para saber posiciones llenas
		vector<int> posUsadas;
		vector<int> posRegistros;
		///////

		/////variables de tabla y columna
		ColumnBlock columnas;
		int posDataColumna;
		registerBlock registroI;
		tableBlock tabla;
		int posDataTabla;
		blockNormal tmp;
		tableBlock tablaUsando;

		archivo.seekg(0, ios::beg);
		archivo.read(reinterpret_cast<char*>(&infromacion_base), sizeof(metadata));
		archivo.read(reinterpret_cast<char*>(&bit), sizeof(bitmap));

		const int SHIFT = 8 * sizeof(char) - 1;
		const char MASK = 1 << SHIFT;

		///saber posiciones ocupadas
		for (int i = 0; i < infromacion_base.cant_blocks / 8; i++)
		{
			char value;
			value = bit.bitmap[i];
			for (int c = 1; c <= SHIFT + 1; c++)
			{
				char z = -1;
				(value & MASK ? z = '1' : z = '0');
				if (z == '1')
				{

					posUsadas.push_back({ (8 * i) + (c - 1) });
				}
				value <<= 1;
			}
		}
		///saber posiciones ocupadas

		//encontrar pos de tabla a insertar
		for (int v : posUsadas)
		{
			archivo.seekg(pointer + (sizeof(blockNormal) * v));
			archivo.read(reinterpret_cast<char*>(&tabla), sizeof(tableBlock));

			if (tabla.type == 'T')
			{

				for (int i = 0; i < infromacion_base.cant_registrosTabla; i++)
				{
					if (tabla.data[i].used == true)
					{
						if (strcmp(tabla.data[i].name, name) == 0)
						{
							tabla.data[i].cant_Columnas;
							posDataTabla = i;
							tablaUsando = tabla;
							posDataColumna = v;
							break;
						}
					}
				}
			}
		}
		Columna* columnasUsando = new Columna[tablaUsando.data[posDataTabla].cant_Columnas];
		//encontrar pos de columna a insertar
		int cont = 0;
		for (int v : posUsadas)
		{

			archivo.seekg(pointer + (sizeof(blockNormal) * v));
			archivo.read(reinterpret_cast<char*>(&columnas), sizeof(ColumnBlock));
			if (columnas.type == 'C')
			{
				for (int i = 0; i < infromacion_base.cant_registrosColumna; i++)
				{
					if (columnas.data[i].used == true)
					{
						if (columnas.data[i].id == 1)
						{

							columnasUsando[cont] = columnas.data[i];
							cont++;
						}
					}
				}
			}
		}
		///encontras pos columnas

		for (int v : posUsadas)
		{
			registerBlock tmp;
			archivo.seekg(pointer + (sizeof(blockNormal) * v));
			archivo.read(reinterpret_cast<char*>(&tmp), sizeof(ColumnBlock));
			if (tmp.type == 'R')
			{

				if (tmp.data[0].id == tablaUsando.data[posDataTabla].id)
				{

					posRegistros.push_back({ v });
				}
			}
		}

		int erase = NULL;
		double erase1 = NULL;
		char* erase2 = NULL;
		int type = NULL;
		bool found = false;
		int place = -1;
		registerBlock registro_guardar;
		registro paraGuardar;
		paraGuardar.dobles = new doble[tablaUsando.data[posDataTabla].cant_Double];
		paraGuardar.enteros = new entero[tablaUsando.data[posDataTabla].cant_Int];
		paraGuardar.var = new varchar[tablaUsando.data[posDataTabla].cant_varchar];

		for (int j = 0; j < 1; j++)
		{

			cout << "Ingrese valor de columna a Moficiar: " << columnasUsando[j].name << endl;
			;

			if (columnasUsando[j].tipo == 1)
			{
				cin >> erase;
				type = columnasUsando[j].tipo;
			}
			if (columnasUsando[j].tipo == 2)
			{
				cin >> erase1;
				type = columnasUsando[j].tipo;
			}
			if (columnasUsando[j].tipo == 3)
			{
				do
				{
					cout << "Ingrese nuevo valor del varchar" << endl;
					cin >> erase2;
				} while (strlen(erase2) > columnasUsando[j].size_if_varchar);
				type = columnasUsando[j].tipo;
			}
		}

		int tmpInt = 0;
		int tmpDouble = 0;
		int tmpChar = 0;
		char tipo[50];

		for (int i = 0; i < posRegistros.size(); i++)
		{
			archivo.seekg(pointer + (sizeof(registerBlock) * posRegistros[i]));
			archivo.read(reinterpret_cast<char*>(&registroI), sizeof(registerBlock));

			for (int k = 0; k < tablaUsando.data[posDataTabla].cant_Registro; k++)
			{

				if (registroI.data[k].used == true)
				{
					int tmpInt = 0;
					int tmpDouble = 0;
					int tmpChar = 0;
					char tipo[50];

					for (int p = 0; p < tablaUsando.data[posDataTabla].cant_Registro; p++)
					{
						if (registroI.data[p].used == true)
						{

							if (type == 1)
							{
								if (registroI.data[p].enteros[0].dato == erase)
								{
									int tmpInt = 0;
									int tmpDouble = 0;
									int tmpChar = 0;
									char tipo[50];
									for (int z = 0; z < tablaUsando.data[posDataTabla].cant_Columnas; z++)
									{

										if (columnasUsando[z].tipo == 1)
										{
											strcpy_s(tipo, "Int");
										}
										if (columnasUsando[z].tipo == 2)
										{
											strcpy_s(tipo, "Double");
										}
										if (columnasUsando[z].tipo == 3)
										{
											strcpy_s(tipo, "Varchar");
										}
										cout << "Ingrese nuevo valor para columna: " << columnasUsando[z].name << " de tipo: " << tipo << endl;
										;
										int entradaInt;
										double entradaDouble = NULL;
										char* entravar = new char[200];

										if (columnasUsando[z].tipo == 1)
										{
											cin >> entradaInt;

											strcpy_s(registroI.data[p].enteros[tmpInt].name, columnasUsando[z].name);
											registroI.data[p].enteros[tmpInt].dato = entradaInt;
											cout << "numero:" << registroI.data[p].enteros[tmpInt].dato << endl;
											tmpInt++;
										}
										if (columnasUsando[z].tipo == 2)
										{
											cin >> entradaDouble;

											strcpy_s(registroI.data[p].dobles[tmpDouble].name, columnasUsando[z].name);
											registroI.data[p].dobles[tmpDouble].dato = entradaDouble;
											tmpDouble++;
										}
										if (columnasUsando[z].tipo == 3)
										{
											do
											{
												cout << "Ingrese valor de varchar" << endl;
												cin >> entravar;
											} while (strlen(entravar) > columnasUsando[z].size_if_varchar);

											strcpy_s(registroI.data[p].var[tmpChar].name, columnasUsando[z].name);
											registroI.data[p].var[tmpChar].dato = entravar;

											tmpChar++;
										}
									}
								}
								if (type == 2)
								{
									if (registroI.data[p].dobles[0].dato == erase1)
									{

										int tmpInt = 0;
										int tmpDouble = 0;
										int tmpChar = 0;
										char tipo[50];
										for (int z = 0; z < tablaUsando.data[posDataTabla].cant_Columnas; z++)
										{

											if (columnasUsando[z].tipo == 1)
											{
												strcpy_s(tipo, "Int");
											}
											if (columnasUsando[z].tipo == 2)
											{
												strcpy_s(tipo, "Double");
											}
											if (columnasUsando[z].tipo == 3)
											{
												strcpy_s(tipo, "Varchar");
											}
											cout << "Ingrese nuevo valor para columna: " << columnasUsando[z].name << " de tipo: " << tipo << endl;
											;
											int entradaInt;
											double entradaDouble = NULL;
											char* entravar = new char[200];

											if (columnasUsando[z].tipo == 1)
											{
												cin >> entradaInt;

												strcpy_s(registroI.data[p].enteros[tmpInt].name, columnasUsando[z].name);
												registroI.data[p].enteros[tmpInt].dato = entradaInt;
												cout << "numero:" << registroI.data[p].enteros[tmpInt].dato << endl;
												tmpInt++;
											}
											if (columnasUsando[z].tipo == 2)
											{
												cin >> entradaDouble;

												strcpy_s(registroI.data[p].dobles[tmpDouble].name, columnasUsando[z].name);
												registroI.data[p].dobles[tmpDouble].dato = entradaDouble;
												tmpDouble++;
											}
											if (columnasUsando[z].tipo == 3)
											{
												do
												{
													cout << "Ingrese valor de varchar" << endl;
													cin >> entravar;
												} while (strlen(entravar) > columnasUsando[z].size_if_varchar);

												strcpy_s(registroI.data[p].var[tmpChar].name, columnasUsando[z].name);
												registroI.data[p].var[tmpChar].dato = entravar;

												tmpChar++;
											}
											registroI.data[p].used = true;
										}

										place = posRegistros[i];
										registro_guardar = registroI;
									}
								}
								if (type == 3)
								{
									if (strcmp(registroI.data[p].var[0].dato, erase2) == 0)
									{

										int tmpInt = 0;
										int tmpDouble = 0;
										int tmpChar = 0;
										char tipo[50];
										for (int z = 0; z < tablaUsando.data[posDataTabla].cant_Columnas; z++)
										{

											if (columnasUsando[z].tipo == 1)
											{
												strcpy_s(tipo, "Int");
											}
											if (columnasUsando[z].tipo == 2)
											{
												strcpy_s(tipo, "Double");
											}
											if (columnasUsando[z].tipo == 3)
											{
												strcpy_s(tipo, "Varchar");
											}
											cout << "Ingrese nuevo valor para columna: " << columnasUsando[z].name << " de tipo: " << tipo << endl;
											;
											int entradaInt;
											double entradaDouble = NULL;
											char* entravar = new char[200];

											if (columnasUsando[z].tipo == 1)
											{
												cin >> entradaInt;

												strcpy_s(registroI.data[p].enteros[tmpInt].name, columnasUsando[z].name);
												registroI.data[p].enteros[tmpInt].dato = entradaInt;
												cout << "numero:" << registroI.data[p].enteros[tmpInt].dato << endl;
												tmpInt++;
											}
											if (columnasUsando[z].tipo == 2)
											{
												cin >> entradaDouble;

												strcpy_s(registroI.data[p].dobles[tmpDouble].name, columnasUsando[z].name);
												registroI.data[p].dobles[tmpDouble].dato = entradaDouble;
												tmpDouble++;
											}
											if (columnasUsando[z].tipo == 3)
											{
												do
												{
													cout << "Ingrese valor de varchar" << endl;
													cin >> entravar;

												} while (strlen(entravar) > columnasUsando[z].size_if_varchar);

												strcpy_s(registroI.data[p].var[tmpChar].name, columnasUsando[z].name);
												registroI.data[p].var[tmpChar].dato = entravar;

												tmpChar++;
											}
											registroI.data[p].used = true;
										}

										place = posRegistros[i];
										registro_guardar = registroI;
									}
								}
							}

							cout << "" << endl;
							break;
						}
					}
				}
				if (found == true)
				{
					break;
				}
			}
		}
		if (place != -1)
		{
			archivo.seekp(pointer + (sizeof(registerBlock) * place));
			archivo.write(reinterpret_cast<const char*>(&registro_guardar), sizeof(registerBlock));
		}
	}

}

int fileSystem::getPosLibreTYC(char file[30], int type)
{
	fstream archivo(file, ios::in | ios::out | ios::binary);
	if (!archivo)
	{
		cout << "error al abrir el archivo" << endl;
	}
	else
	{
		char name[30];
		if (type == 3)
		{
			cout << "Tabla a ingresar" << endl;
			cin >> name;
		}

		metadata infromacion_base;
		bitmap bit;
		int pointer = sizeof(metadata) + sizeof(bitmap);
		int sumpointer = sizeof(blockNormal);
		///Vector para saber posiciones llenas
		vector<int> posUsadas;
		vector<int> posRegistros;
		///////

		/////variables de tabla y columna
		ColumnBlock columnas;
		int posDataColumna = NULL;
		registerBlock registroI;
		tableBlock tabla;
		int posDataTabla = -1;
		blockNormal tmp;
		tableBlock tablaUsando;

		archivo.seekg(0, ios::beg);
		archivo.read(reinterpret_cast<char*>(&infromacion_base), sizeof(metadata));
		archivo.read(reinterpret_cast<char*>(&bit), sizeof(bitmap));

		const int SHIFT = 8 * sizeof(char) - 1;
		const char MASK = 1 << SHIFT;

		///saber posiciones ocupadas
		for (int i = 0; i < infromacion_base.cant_blocks / 8; i++)
		{
			char value;
			value = bit.bitmap[i];
			for (int c = 1; c <= SHIFT + 1; c++)
			{
				char z = -1;
				(value & MASK ? z = '1' : z = '0');
				if (z == '1')
				{

					posUsadas.push_back({ (8 * i) + (c - 1) });
				}
				value <<= 1;
			}
		}
		///saber posiciones ocupadas

		//encontrar pos de tabla a insertar
		if (type == 1 || type == 3)
		{
			for (int v : posUsadas)
			{

				archivo.seekg(pointer + (sizeof(blockNormal) * v));
				archivo.read(reinterpret_cast<char*>(&tabla), sizeof(tableBlock));

				if (tabla.type == 'T')
				{
					//cout << "aquirrr" << endl;
					for (int i = 0; i < infromacion_base.cant_registrosTabla; i++)
					{
						if (tabla.data[i].used == true)
						{
							if (type == 3)
							{
								if (tabla.length < infromacion_base.cant_registrosTabla && strcmp(tabla.data[i].name, name) == 0)
								{
									posDataTabla = i;
									tablaUsando = tabla;
								}
							}

							if (tabla.length < infromacion_base.cant_registrosTabla && type == 1)
							{
								return v;
							}
						}
					}
				}
			}
		}
		else if (type == 2)
		{
			int cont = 0;
			for (int v : posUsadas)
			{
				archivo.seekg(pointer + (sizeof(blockNormal) * v));
				archivo.read(reinterpret_cast<char*>(&columnas), sizeof(ColumnBlock));
				if (columnas.type == 'C')
				{
					if (columnas.length < infromacion_base.cant_registrosColumna)
					{
						return v;
					}
				}
			}
		}
		else if (type == 3)
		{
			//encontrar pos de columna a insertar
			int cont = 0;
			for (int v : posUsadas)
			{
				registerBlock reg;
				archivo.seekg(pointer + (sizeof(blockNormal) * v));
				archivo.read(reinterpret_cast<char*>(&reg), sizeof(registerBlock));
				if (columnas.type == 'R')
				{
					if (columnas.length < infromacion_base.cant_registrosColumna && columnas.data[0].id == tablaUsando.data[posDataTabla].id)
					{
						return v;
					}
				}
			}
		}
		///encontras pos columnas
	}
	return -1;
}

bool fileSystem::findTable(char file[30], char name[30])
{
	fstream archivo(file, ios::in | ios::out | ios::binary);
	if (!archivo)
	{
		cout << "Error with file" << endl;
	}
	else
	{
		metadata infromacion_base;
		bitmap bit;
		int pointer = sizeof(metadata) + sizeof(bitmap);
		int sumpointer = sizeof(blockNormal);
		///Vector para saber posiciones llenas
		vector<int> posUsadas;
		vector<int> posRegistros;
		///////

		/////variables de tabla y columna
		ColumnBlock columnas;
		int posDataColumna;
		registerBlock registroI;
		tableBlock tabla;
		int posDataTabla;
		blockNormal tmp;
		tableBlock tablaUsando;

		archivo.seekg(0, ios::beg);
		archivo.read(reinterpret_cast<char*>(&infromacion_base), sizeof(metadata));
		archivo.read(reinterpret_cast<char*>(&bit), sizeof(bitmap));

		const int SHIFT = 8 * sizeof(char) - 1;
		const char MASK = 1 << SHIFT;

		///saber posiciones ocupadas
		for (int i = 0; i < infromacion_base.cant_blocks / 8; i++)
		{
			char value;
			value = bit.bitmap[i];
			for (int c = 1; c <= SHIFT + 1; c++)
			{
				char z = -1;
				(value & MASK ? z = '1' : z = '0');
				if (z == '1')
				{

					posUsadas.push_back({ (8 * i) + (c - 1) });
				}
				value <<= 1;
			}
		}
		///saber posiciones ocupadas

		//encontrar pos de tabla a insertar
		for (int v : posUsadas)
		{

			archivo.seekg(pointer + (sizeof(blockNormal) * v));
			archivo.read(reinterpret_cast<char*>(&tabla), sizeof(tableBlock));

			if (tabla.type == 'T')
			{

				for (int i = 0; i < infromacion_base.cant_registrosTabla; i++)
				{
					if (tabla.data[i].used == true)
					{

						if (strcmp(tabla.data[i].name, name) == 0)
						{
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

char* fileSystem::initBitMap(int DB_AMOUNT)
{
	char* bitMap = new char[DB_AMOUNT / 8];
	for (int i = 0; i < DB_AMOUNT / 8; i++)
	{
		bitMap[i] = 0;
	}
	return bitMap;
}

int fileSystem::getPosLibre(char* bitMap, int DB_AMOUNT)
{
	const int SHIFT = 8 * sizeof(char) - 1;
	const char MASK = 1 << SHIFT;

	for (int i = 0; i < DB_AMOUNT / 8; i++)
	{
		char value;
		value = bitMap[i];
		for (int c = 1; c <= SHIFT + 1; c++)
		{
			char z = 'x';
			(value & MASK ? z = '1' : z = '0');

			if (z == '0')
			{
				return (8 * i) + (c - 1);
			}

			value <<= 1;

			if (c % 8 == 0)
				cout << ' ';
		}
	}
	cout << endl;
	return -1;
}

char* fileSystem::setOn(char* bitMap, int nBlock)
{
	int positionByte = nBlock / 8;
	int iniPosition = (nBlock / 8) * 8;

	for (int i = iniPosition, x = 7; i < (positionByte * 8) + 8; i++, x--)
	{
		if (i == nBlock)
		{
			bitMap[positionByte] |= 1 << x;
			break;
		}
	}
	return bitMap;
}

char* fileSystem::setOf(char* bitMap, int nBlock)
{
	int positionByte = nBlock / 8;
	int iniPosition = (nBlock / 8) * 8;

	for (int i = iniPosition, x = 7; i < (positionByte * 8) + 8; i++, x--)
	{
		if (i == nBlock)
		{
			bitMap[positionByte] &= ~(1 << x);
			break;
		}
	}
	return bitMap;
}

int fileSystem::getId(char file[30])
{
	int id = 0;
	fstream archivo(file, ios::in | ios::out | ios::binary);
	if (!archivo)
	{
		cout << "no se puede abrir el archivo" << endl;
	}
	else
	{
		metadata info;
		archivo.seekg(0, ios::beg);
		archivo.read(reinterpret_cast<char*>(&info), sizeof(metadata));

		id = info.id_tablas + 1;
		info.id_tablas += 1;
		archivo.seekp(0, ios::beg);
		archivo.write(reinterpret_cast<const char*>(&info), sizeof(metadata));
	}
	archivo.close();
	return id;
}

int fileSystem::decidePos(char file[30], int type)
{
	fstream archivo(file, ios::in | ios::out | ios::binary);
	if (!archivo)
	{
		cout << "Error with file" << endl;
	}
	else
	{
		bitmap bit;
		metadata informacion_base;
		archivo.seekg(0, ios::beg);
		archivo.read(reinterpret_cast<char*>(&informacion_base), sizeof(metadata));
		archivo.read(reinterpret_cast<char*>(&bit), sizeof(bitmap));

		int libre = getPosLibre(bit.bitmap, informacion_base.cant_blocks);
		int libre2 = getPosLibreTYC(file, type);

		if (libre2 == -1)
		{

			setOn(bit.bitmap, libre);
			writebitMap(file, bit.bitmap);
			return libre;
		}
		else if (libre2 == -1 && libre == -1)
		{

			return -1;
		}
		else
		{

			return libre2;
		}
	}
}

void fileSystem::writebitMap(char file[30], char* bitmap1)
{
	fstream archivo(file, ios::in | ios::out | ios::binary);
	if (!archivo)
	{
		cout << "Error con archivo" << endl;
	}
	else
	{
		bitmap bit;
		bit.bitmap = bitmap1;
		archivo.seekp(sizeof(metadata), ios::beg);
		archivo.write(reinterpret_cast<const char*>(&bit), sizeof(bitmap));
	}
}
