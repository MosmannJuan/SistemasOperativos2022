/*
 * kernel.h
 *
 *  Created on: 14 abr. 2022
 *      Author: utnso
 */

#ifndef KERNEL_H_
#define KERNEL_H_
#include <commons/config.h>
#include <commons/log.h>
#include <string.h>

char* ipMemoria;
char* ipKernel;
int puertoMemoria;
char* ipCpu;
char* algoritmoPlanificacion;
int puertoCpuInterrupt;
int puertoCpuDispatch;
char* puertoEscucha;
int estimacionInicial;
int alfa;
int gradoMultiprogramacion;
int tiempoMaximoBloqueado;


typedef struct
{
	int conn_kernel;
	//int conn_memoria;
	//char *ipMemoria;
	//char *puertoMemoria;
} conexiones;

t_config* kernel_config;

#endif /* KERNEL_H_ */

