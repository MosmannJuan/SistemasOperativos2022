#ifndef MEMORIA_H_
#define MEMORIA_H_


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/string.h>
#include <stdint.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <unistd.h>
#include "../utils.h"


t_config* memoria_config;
#endif /* MEMORIA_H_ */

/////FIJARSE SI CAMBIAR A uint32_t
char* ipMemoria;
char* puertoEscucha;
int tam_memoria;
int tam_pagina;
int tamTabla;
int entradas_por_tabla;
int retardoMemoria;
int marcosPorProceso;
int retardoSwap;
void* baseMemoria;
char* pathSwap;
t_list* memoriaPrimerNivelList;
pthread_mutex_t mutexMemoria;
t_log *loggerMemoria;
char* algoritmoReemplazo;
char* pathSwap;
t_list* tablas_primer_nivel;
t_list* tablas_segundo_nivel;
void* base_memoria;
t_list*  marcos_disponibles;
int conexion_kernel;
int conexion_cpu;
pthread_t *hilo_kernel_handler;
int conexion;

typedef struct
{
	unsigned int id_segundo_nivel;
} entrada_primer_nivel;

typedef struct
{
	uint32_t marco;
	bool presencia;
	bool uso;
	bool modificado;
} entrada_segundo_nivel;


typedef enum {
	INICIALIZAR_ESTRUCTURAS
}accion_memoria;

void abrirArchivoConfiguracion();
void configurarMemoria();
void atenderMensajes();
void borrar(int pagina, int marco);
void crear_archivo_swap(unsigned int tamanio_proceso, unsigned int pid);
int calcular_cantidad_tablas_necesarias(unsigned int tamanio_proceso);
void inicializar_listas_procesos();
void inicializar_marcos_disponibles();
void* conexion_kernel_handler(void* args);
void inicializar_kernel_handler(pthread_t *hilo_kernel_handler);
