#ifndef PARSER_H_
#define PARSER_H_

#include "utils.h"

// ---- FUNCIONES ----//

void leer_y_enviar_archivo_de_instrucciones(char * pathArchivoInstrucciones, t_log * logger, int conexion);
void lectura_y_asignacion_parametros(Instruccion * instruccionAux, FILE * file);
void lectura_y_asignacion_un_parametro(Instruccion * instruccionAux, FILE * file, int i);
void lectura_y_asignacion_dos_parametro(Instruccion * instruccionAux, FILE * file, int i);

#endif /* PARSER_H_ */