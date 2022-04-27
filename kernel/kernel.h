
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
int estimacionInicial;
int alfa;
int gradoMultiprogramacion;
int tiempoMaximoBloqueado;



typedef struct {
	unsigned int id;
	unsigned int tam_proceso;
	t_list * intrucciones;
	unsigned int pc;
	/* TABLA PAGINAS ??? */
	double rafaga;

}pcb;

//Listas de estados

t_list * bloqueado;
t_list * ready;
t_list * running;
t_list * bloqueado_suspendido;
t_list * ready_suspendido;


t_config * kernel_config;

void incilizar_pcb(pcb pcb);

#endif /* KERNEL_H_ */
