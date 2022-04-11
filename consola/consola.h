#ifndef CONSOLA_H_
#define CONSOLA_H_
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <commons/config.h>

#include "utils.h"

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

int conexion;
char* ip;
char* puertoKernel;
char* pathArchivoInstrucciones;
t_list* listaInstrucciones;
t_config* consola_config;
t_log* logger;


#endif /* CONSOLA_H_ */

