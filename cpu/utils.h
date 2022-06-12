#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <unistd.h>

// ---- VARIABLES ----//
t_log * logger;
double contador_rafaga;

// ---- ESTRUCTURAS Y ENUMS ----//

typedef enum {
	NO_OP,
	I_O,
	READ,
	WRITE,
	COPY,
	EXIT
}TipoInstruccion;

typedef enum{
	PASAR_A_BLOQUEADO,
	PASAR_A_READY,
	PASAR_A_EXIT,
	EVALUAR_DESALOJO,
	EJECUTAR
} mensaje_cpu;

typedef struct {
	unsigned int id;
	unsigned int tam_proceso;
	t_list * instrucciones;
	unsigned int pc;
	int tabla_paginas;
	double rafaga;

}pcb;

typedef struct Instruccion {
	TipoInstruccion tipo;
	unsigned int params[2];
}Instruccion;

typedef struct {
	void* datos;
	mensaje_cpu mensaje;
} mensaje_dispatch;

typedef struct {
	pcb* pcb_a_bloquear;
	double rafaga_real_anterior;
	unsigned int tiempo_bloqueo; //Esta sería la estructura correcta a recibir en el void* datos en el caso de que se envíe a bloquear un proceso
} bloqueo_pcb;

typedef struct {
	pcb* pcb_a_interrumpir;
	double rafaga_real_anterior;
} interrupcion_pcb;

// ---- FUNCIONES ----//

pcb * pcb_create();
pcb * recibir_pcb(int socket_cliente);
void leer_y_asignar_pcb(int socket_cliente, pcb* pcb_leido);
int conexion_servidor(char * ip, char * puerto);
int iniciar_servidor(char * , char * );
int esperar_cliente(int);
void enviar_exit(int socket_cliente);
void* serializar_mensaje_bloqueo(pcb* pcb_a_enviar, unsigned int tiempo_bloqueo, int bytes);
void enviar_pcb_bloqueo(pcb* pcb_a_enviar, unsigned int tiempo_bloqueo, int socket_cliente);
void terminar_programa(int conexionA, int conexionB, int conexionC, t_log * logger, t_config * config);
void* serializar_pcb(pcb* pcb_a_enviar, void* memoria_asignada, int desplazamiento);
void serializar_instrucciones(void* memoria_asignada, int desplazamiento, t_list* instrucciones);
void enviar_pcb_interrupcion(pcb* pcb_a_enviar, int socket_cliente);
void* serializar_mensaje_interrupcion(pcb* pcb_a_enviar, int bytes);
#endif /* UTILS_H_ */
