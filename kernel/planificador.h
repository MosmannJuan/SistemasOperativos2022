#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include "instrucciones_handler.h"
#include <semaphore.h>
#include <string.h>

// ---- VARIABLES ----//

sem_t semaforo_pid;
sem_t semaforo_pid_comparacion;
sem_t semaforo_lista_new_add;
sem_t semaforo_lista_new_remove;
sem_t semaforo_lista_ready_add;
int alfa;
char * algoritmoPlanificacion;
unsigned int pid_comparacion;
unsigned int estimacion_inicial;


// ---- ESTRUCTURAS Y ENUMS ----//
typedef enum{
	PASAR_A_BLOQUEADO,
	PASAR_A_READY
} mensaje_cpu;

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
} argumentos_largo_plazo;

typedef struct {
	unsigned int tiempo_bloqueo;
	pcb * pcb_actualizado;
} argumentos_hilo_bloqueo;

typedef struct {
	pcb * pcb_actualizado;
	unsigned int tiempo_bloqueo;
	mensaje_cpu mensaje;
} mensaje_dispatch;

// ---- LISTA DE ESTADOS ----//

t_list * new;
t_list * bloqueado;
t_list * ready;
t_list * running;
t_list * bloqueado_suspendido;
t_list * ready_suspendido;
t_list * exit_estado;


// ---- FUNCIONES ----//

pcb * inicializar_pcb(t_list * lista_instrucciones, unsigned int tam_proceso);
pcb* pcb_create();
void pcb_destroy(pcb * pcb);
void * hilo_de_largo_plazo (void * args);
void inicializar_listas_procesos();
void * hilo_de_largo_plazo (void * args_p);
void * hilo_bloqueo_proceso (void* args_p);
bool es_pid_a_desbloquear(void * pcb);
void * hilo_de_corto_plazo_fifo_ready(void* argumentos);
void * hilo_de_corto_plazo_fifo_running(void* argumentos);
unsigned int calcular_estimacion_rafaga();
bool ordenar_por_estimacion_rafaga(void * unPcb, void* otroPcb);
void * hilo_de_corto_plazo_sjf_running(void* argumentos);

#endif /* PLANIFICADOR_H_ */
