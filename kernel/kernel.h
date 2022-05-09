
#ifndef KERNEL_H_
#define KERNEL_H_

#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "instrucciones_handler.h"


char * ipMemoria;
char * puertoMemoria;
char * ipKernel;
char * puertoEscucha;
char * ipCpu;
char * puertoCpuInterrupt;
char * puertoCpuDispatch;
int gradoMultiprogramacion;
int tiempoMaximoBloqueado;

int conexion_con_memoria;

int conexionConsola;
int conexionDispatch;
int conexionInterrupt;

t_config * kernel_config;


void inicializar_semaforos();
void inicializar_planificador_corto_plazo(pthread_t * hilo_ready, pthread_t * hilo_running);
void inicializar_planificador_largo_plazo(pthread_t * hiloNewReady, pthread_t  * hilo_exit);

#endif /* KERNEL_H_ */
