#ifndef CPU_H_
#define CPU_H_


//#include <sharedUtils.c>
#include <commons/log.h>
#include <stdint.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include "utils.h"

bool detener_ejecucion;
int interrupciones = 0;
int	entradasTlb ;
char*	reemplazoTlb;
int	retardoNoop;
char*	ipMemoria;
char*	puertoMemoria;
char* 	ipKernel;
char*	puertoEscuchaDispatch;
char*	puertoEscuchaInterrupt;

sem_t *sem_interrupcion;
sem_t *sem_dispatch;


int conexionMemoria;
int conexionDispatch;
int conexionInterrupt;

t_config* cpu_config;
t_log * cpuLogger;

typedef struct DireccionLogica{
	int primerNivel;
	int segundoNivel;
	int desplazamiento;
}DireccionLogica;


DireccionLogica* fetch_operands(unsigned int* operandos);
void* decode (pcb* pcb_decode, Instruccion * instruccion_decode);
void* fetch(pcb* pcb_fetch);
void abrirArchivoConfiguracion();
void ciclo(pcb* paquetePcb);
void ejecutar_NO_OP(unsigned int parametro);
void ejecutar_I_O(pcb* pcb_a_bloquear, unsigned int tiempo_bloqueo);
void ejecutar_exit();


#endif /* CPU_H_ */
