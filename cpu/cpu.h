#ifndef CPU_H_
#define CPU_H_

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


//---------------------------------------------------------------
// ----------------- ENUMS Y VARIABLES GLOBALES  ----------------
//---------------------------------------------------------------

typedef enum {
	OBTENER_TABLA_SEGUNDO_NIVEL,
	OBTENER_NUMERO_MARCO,
	LEER,
	ESCRIBIR
} mensaje_memoria;


char*	reemplazoTlb;
char*	ipMemoria;
char*	puertoMemoria;
char* 	ipKernel;
char*	puertoEscuchaDispatch;
char*	puertoEscuchaInterrupt;
int entradas_por_tabla;
int tamanio_pagina;
int conexion_memoria;
int conexion_dispatch;
int conexion_interrupt;
int	entradasTlb ;
int	retardoNoop;
bool detener_ejecucion;
bool contador_rafaga_inicializado;
bool hay_interrupciones;
sem_t *sem_interrupcion;
sem_t *sem_dispatch;
t_config* cpu_config;
t_log * cpu_logger;
t_log* cpu_info_logger;
t_list* tlb;



//---------------------------------------------------------------
// ----------------- DECLARACION DE FUNCIONES  ------------------
//---------------------------------------------------------------

void* contador(void* args);
void* interrupcion_handler(void* args);
void* decode (pcb* pcb_decode, Instruccion * instruccion_decode);
void* fetch(pcb* pcb_fetch);
void abrirArchivoConfiguracion();
void ciclo(pcb* paquetePcb);
void ejecutar_NO_OP(unsigned int parametro);
void ejecutar_I_O(pcb* pcb_a_bloquear, unsigned int tiempo_bloqueo);
void ejecutar_exit();
void inicializar_hilo_conexion_interrupt(pthread_t* hilo_interrupcion_handler);
void atender_interrupcion(pcb* pcb_interrumpido);
double mmu(unsigned int dir_logica, int numero_tabla_primer_nivel);
void ejecutar_WRITE(unsigned int direccion_logica, unsigned int valor_a_escribir, int tabla_paginas);
unsigned int fetch_operands(unsigned int direccion_logica, int tabla_paginas);
void ejecutar_READ(unsigned int direccion_logica, int tabla_paginas);
void recibir_valores_globales_memoria();

#endif /* CPU_H_ */
