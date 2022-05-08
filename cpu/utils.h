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

// ---- ESTRUCTURAS Y ENUMS ----//

typedef enum {
	NO_OP,
	I_O,
	READ,
	WRITE,
	COPY,
	EXIT
}TipoInstruccion;

typedef struct {
	unsigned int id;
	unsigned int tam_proceso;
	t_list * instrucciones;
	unsigned int pc;
	/* TABLA PAGINAS ??? */
	double rafaga;

}pcb;

typedef struct Instruccion {
	TipoInstruccion tipo;
	unsigned int params[2];
}Instruccion;

// ---- FUNCIONES ----//

pcb * pcb_create();
pcb * recibir_pcb(int socket_cliente);
void leer_y_asignar_pcb(int socket_cliente, pcb* pcb_leido);
int conexion_servidor(char * ip, char * puerto);
int iniciar_servidor(char * , char * );
int esperar_cliente(int);
void recibir_mensaje(int);
int esperar_cliente(int);
void terminar_programa(int conexionA, int conexionB, int conexionC, t_log * logger, t_config * config);

#endif /* UTILS_H_ */
