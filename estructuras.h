#pragma once
#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include "fileType.h"
#include "Datatype.h"
#include <vector>

using namespace std;

struct metadata {

	char name[30];
	int gb_mb;
	double sizebytes;
	int size_blocks;
	int cant_blocks;
	int cant_registrosColumna;
	int cant_registrosTabla;
	int id_tablas;
	int residuo_tablas;
	int residuo_Columnas;
};
struct bitmap {

	char* bitmap;
};
struct table
{
	int id;
	char name[30];
	int cant_Columnas;
	int cant_Int;
	int cant_Double;
	int cant_varchar;
	int cant_Registro;
	int residuoRegistro;
	bool used = false;
};
struct entero
{
	char name[30];
	int dato;
};
struct doble
{
	char name[30];
	double dato;
};
struct varchar
{
	char name[30];
	char* dato;
};
struct registro
{
	int id;
	entero* enteros;
	doble* dobles;
	varchar* var;
	bool used = false;
};
struct Columna
{
	int id;
	char name[30];
	int tipo;//1-Int 2-Double 3-varchar
	int size_if_varchar;
	bool used = false;
};
struct blockNormal {
	char* dataInicio;
	table* data;
	char* dataFinal;
	int pointer;
	char type = 'N';
	int length = 0; //cantidad de registros
};
struct tableBlock {
	char* dataInicio;
	table* data;
	char* dataFinal;
	int pointer;
	char type = 'T'; 
	int length = 0;

};
struct registerBlock {
	char* dataInicio;
	registro* data;
	char* dataFinal;
	int pointer;
	char type = 'R';
	int length = 0;

};
struct ColumnBlock {

	char* dataInicio;
	Columna* data;
	char* dataFinal;
	int pointer;
	char type = 'C';
	int length = 0;
};

#endif // !ESTRUCTURAS_H
