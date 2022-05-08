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

typedef enum {
	BLOQUEADO,
	INTERRUPCION,
	FINALIZADO
}EstadoPcb;

typedef struct {
	unsigned int id;
    unsigned int tam_proceso;
	t_list * instrucciones;
	unsigned int pc;
	/* TABLA PAGINAS ??? */
	double rafaga;
}Pcb;

typedef struct {
	Pcb *pcb;
	EstadoPcb estado;

}PaquetePcb;

typedef struct DireccionLogica{
	int primerNivel;
	int segundoNivel;
	int desplazamiento;
}DireccionLogica;

DireccionLogica* fetch_operands(unsigned int* operandos);
void* decode_execute (PaquetePcb * pcb_decode, Instruccion * instruccion_decode);
void* fetch(Pcb * pcb_fetch);
void abrirArchivoConfiguracion();
void ciclo(PaquetePcb *paquetePcb);


#endif /* CPU_H_ */
