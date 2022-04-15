/*
 * consola.h
 *
 *  Created on: 14 abr. 2022
 *      Author: utnso
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_
#include <commons/config.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>

typedef enum {
	NO_OP,
	I_O,
	READ,
	WRITE,
	COPY,
	EXIT
} TipoInstruccion;

typedef struct Instruccion {
	TipoInstruccion tipo;
	unsigned int params[2];
} Instruccion;


char* ip;
char* puertoKernel;
t_config* consola_config;
char* pathArchivoInstrucciones;
t_list* listaInstrucciones;

#endif /* CONSOLA_H_ */

