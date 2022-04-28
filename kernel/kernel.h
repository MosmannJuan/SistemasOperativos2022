
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
char * ipKernel;
int puertoMemoria;
char * ipCpu;
char * algoritmoPlanificacion;
int puertoCpuInterrupt;
int puertoCpuDispatch;
char * puertoEscucha;
unsigned int estimacionInicial;
int alfa;
int gradoMultiprogramacion;
int tiempoMaximoBloqueado;

t_config * kernel_config;
//sem_t semaforo_pid;


#endif /* KERNEL_H_ */
