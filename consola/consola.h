#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <string.h>

#include <stdio.h>

#include <stdlib.h>

#include <commons/collections/list.h>

#include <commons/log.h>

#include <commons/config.h>

#include <commons/string.h>

#include "utils.h"

typedef enum {
  NO_OP,
  I_O,
  READ,
  WRITE,
  COPY,
  EXIT
}
TipoInstruccion;

typedef struct Instruccion {
  TipoInstruccion tipo;
  unsigned int params[2];
}
Instruccion;

int conexion;
char * ip;
char * puertoKernel;
char * pathArchivoInstrucciones;
t_list * listaInstrucciones;
t_config * consola_config;
t_log * logger;

Instruccion * crear_instruccion();
void lectura_y_asignacion_parametros(Instruccion * instruccionAux, FILE * file);
void lectura_y_asignacion_un_parametro(Instruccion * instruccionAux, FILE * file, int i);
void lectura_y_asignacion_dos_parametro(Instruccion * instruccionAux, FILE * file, int i);

#endif /* CONSOLA_H_ */
