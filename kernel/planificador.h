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
sem_t semaforo_lista_ready_remove;
sem_t semaforo_lista_ready_suspendido_remove;
sem_t semaforo_lista_ready_suspendido_add;
sem_t semaforo_lista_running_remove;
sem_t semaforo_grado_multiprogramacion;

int alfa;
char * algoritmoPlanificacion;
unsigned int pid_comparacion;
unsigned int estimacion_inicial;
int tiempoMaximoBloqueado;
unsigned int limite_grado_multiprogramacion;

// ---- SOCKETS ----//
int dispatch;
int interrupt;


// ---- ESTRUCTURAS Y ENUMS ----//
typedef enum{
	PASAR_A_BLOQUEADO,
	PASAR_A_READY,
	PASAR_A_EXIT,
	EVALUAR_DESALOJO
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
} mensaje_dispatch; //Dejo definida esta estructura para que compile mientras hago el refactor

typedef struct {
	void* datos;
	mensaje_cpu mensaje;
} mensaje_dispatch_posta; //TODO: Renombrar luego del refactor

typedef struct {
	pcb* pcb_a_bloquear;
	unsigned int tiempo_bloqueo; //Esta sería la estructura correcta a recibir en el void* datos en el caso de que se envíe a bloquear un proceso
} bloqueo_pcb;


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
void * hilo_new_ready (void* args);
void * hilo_de_largo_plazo (void * args);
void inicializar_listas_procesos();
void * hilo_pcb_new (void * args_p);
void * hilo_bloqueo_proceso (void* args_p);
bool es_pid_a_desbloquear(void * pcb);
bool ordenar_por_estimacion_rafaga(void * unPcb, void* otroPcb);
void * hilo_de_corto_plazo_fifo_ready(void* argumentos);
void * hilo_de_corto_plazo_fifo_running(void* argumentos);
unsigned int calcular_estimacion_rafaga();
void * hilo_de_corto_plazo_sjf_ready(void* argumentos);
void * hilo_de_corto_plazo_sjf_running(void* argumentos);
void * exit_largo_plazo(void * argumentos);
void * cpu_dispatch_handler(void * argumentos);
mensaje_dispatch_posta* recibir_mensaje_dispatch();
void enviar_pcb(pcb* pcb_a_enviar, int socket_cliente);
void serializar_instrucciones(void* memoria_asignada, int desplazamiento, t_list* instrucciones);
void* serializar_pcb(pcb* pcb_a_enviar, int bytes);

#endif /* PLANIFICADOR_H_ */
