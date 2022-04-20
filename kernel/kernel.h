
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
int puertoCpuInterrupt;
int puertoCpuDispatch;
int gradoMultiprogramacion;
int tiempoMaximoBloqueado;
int conexion_con_memoria;

t_config * kernel_config;


void inicializar_semaforos();
void inicializar_planificador_corto_plazo(pthread_t * hilo_ready, pthread_t * hilo_running);


#endif /* KERNEL_H_ */
