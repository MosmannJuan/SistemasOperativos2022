#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include "instrucciones_handler.h"

// ---- ESTRUCTURAS Y ENUMS ----//

typedef struct {
	unsigned int id;
	unsigned int tam_proceso;
	t_list * instrucciones;
	unsigned int pc;
	/* TABLA PAGINAS ??? */
	double rafaga;

}pcb;

typedef struct {
	t_list* instrucciones;
	unsigned int tam_proceso;
	double estimacion_rafaga;
} argumentos_largo_plazo;

// ---- LISTA DE ESTADOS ----//

t_list * new;
t_list * bloqueado;
t_list * ready;
t_list * running;
t_list * bloqueado_suspendido;
t_list * ready_suspendido;
t_list * exit_estado;


// ---- FUNCIONES ----//

pcb * inicializar_pcb(t_list *, unsigned int, double);
pcb* pcb_create();
void pcb_destroy(pcb * pcb);
void * hilo_de_largo_plazo (void * args);
void inicializar_listas_procesos();
void * hilo_de_largo_plazo (void * args_p);

#endif /* PLANIFICADOR_H_ */
