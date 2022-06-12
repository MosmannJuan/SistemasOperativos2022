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
#include <math.h>
#include <pthread.h>
#include "utils.h"

typedef enum {
	OBTENER_TABLA_SEGUNDO_NIVEL,
	OBTENER_NUMERO_MARCO,
	LEER,
	ESCRIBIR,
	SOLICITAR_VALORES_GLOBALES
} mensaje_memoria;

bool detener_ejecucion;
int	entradasTlb ;
char*	reemplazoTlb;
int	retardoNoop;
char*	ipMemoria;
char*	puertoMemoria;
char* 	ipKernel;
char*	puertoEscuchaDispatch;
char*	puertoEscuchaInterrupt;
int entradas_por_tabla;
int tamanio_pagina;
bool contador_rafaga_inicializado;
bool hay_interrupciones;


sem_t *sem_interrupcion;
sem_t *sem_dispatch;

int conexion_memoria;
int conexion_dispatch;
int conexion_interrupt;

t_config* cpu_config;
t_log * cpuLogger;
t_log* cpu_info_logger;

void* decode (pcb* pcb_decode, Instruccion * instruccion_decode);
void* fetch(pcb* pcb_fetch);
void abrirArchivoConfiguracion();
void ciclo(pcb* paquetePcb);
void ejecutar_NO_OP(unsigned int parametro);
void ejecutar_I_O(pcb* pcb_a_bloquear, unsigned int tiempo_bloqueo);
void ejecutar_exit();
double mmu(unsigned int dir_logica, int numero_tabla_primer_nivel);
void* conexion_memoria_handler(void*);
void inicializar_hilo_conexion_memoria(pthread_t* hilo_conexion_memoria);
void* contador(void* args);
void* interrupcion_handler(void* args);
void inicializar_hilo_conexion_interrupt(pthread_t* hilo_interrupcion_handler);
void atender_interrupcion(pcb* pcb_interrumpido);

#endif /* CPU_H_ */
