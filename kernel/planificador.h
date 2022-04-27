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

// ---- LISTA DE ESTADOS ----//


t_list * bloqueado;
t_list * ready;
t_list * running;
t_list * bloqueado_suspendido;
t_list * ready_suspendido;


// ---- FUNCIONES ----//

pcb * inicializar_pcb(t_list *, unsigned int, double);
pcb* pcb_create();
void pcb_destroy(pcb * pcb);


#endif /* PLANIFICADOR_H_ */
