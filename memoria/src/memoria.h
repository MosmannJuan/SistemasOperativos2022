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


//---------------------------------------------------------------
// ----------------- ENUMS Y VARIABLES GLOBALES  ----------------
//---------------------------------------------------------------

int conexion_kernel;
int conexion_cpu;
int conexion;
int tam_memoria;
int tam_pagina;
int entradas_por_tabla;
int retardo_memoria;
int marcos_por_proceso;
int retardo_swap;
char* ip_memoria;
char* algoritmo_reemplazo;
char* path_swap;
char* puerto_escucha;
void* base_memoria;
t_list* tablas_primer_nivel;
t_list* tablas_segundo_nivel;
t_list*  marcos_disponibles;
t_log* logger_memoria;
pthread_t *hilo_kernel_handler;






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
}accion_memoria_con_kernel;


typedef enum {
	OBTENER_TABLA_SEGUNDO_NIVEL,
	OBTENER_NUMERO_MARCO,
	LEER,
	ESCRIBIR,
	SOLICITAR_VALORES_GLOBALES
} accion_memoria_con_cpu;



//---------------------------------------------------------------
// ----------------- DECLARACION DE FUNCIONES  ------------------
//---------------------------------------------------------------

int calcular_cantidad_tablas_necesarias(unsigned int tamanio_proceso);
void inicializar_kernel_handler(pthread_t *hilo_kernel_handler);
void inicializar_listas_procesos();
void inicializar_marcos_disponibles();
void abrirArchivoConfiguracion();
void configurarMemoria();
void atenderMensajes();
void inicializar_cpu_handler(pthread_t *hilo_cpu_handler);
void enviar_globales();
void borrar(int pagina, int marco);
void crear_archivo_swap(unsigned int tamanio_proceso, unsigned int pid);
void* conexion_kernel_handler(void* args);
void* conexion_cpu_handler(void* args);
unsigned int ejecutar_lectura(double dir_fisica);
int ejecutar_escritura(double dir_fisica, unsigned int valor_escritura);
