#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <sys/socket.h>

#include <netdb.h>

#include <commons/log.h>

#include <commons/config.h>

t_log * logger;

int iniciar_servidor(char * , char * );
int esperar_cliente(int);
void recibir_mensaje(int);
int esperar_cliente(int);
void terminar_programa(int conexion, t_log * logger, t_config * config);

#endif
