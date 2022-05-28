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

typedef enum{
	PASAR_A_BLOQUEADO,
	PASAR_A_READY,
	PASAR_A_EXIT,
	EVALUAR_DESALOJO
} mensaje_cpu;

typedef struct DireccionLogica{
	int primerNivel;
	int segundoNivel;
	int desplazamiento;
}DireccionLogica;

typedef struct {
	void* datos;
	mensaje_cpu mensaje;
} mensaje_dispatch_posta; //TODO: Renombrar luego del refactor

typedef struct {
	pcb* pcb_a_bloquear;
	double rafaga_real_anterior;
	unsigned int tiempo_bloqueo; //Esta sería la estructura correcta a recibir en el void* datos en el caso de que se envíe a bloquear un proceso
} bloqueo_pcb;

typedef struct {
	pcb* pcb_a_interrumpir;
	double rafaga_real_anterior;
} interrupcion_pcb;

DireccionLogica* fetch_operands(unsigned int* operandos);
void* decode_execute (pcb* pcb_decode, Instruccion * instruccion_decode);
void* fetch(pcb* pcb_fetch);
void abrirArchivoConfiguracion();
void ciclo(pcb* paquetePcb);


#endif /* CPU_H_ */
