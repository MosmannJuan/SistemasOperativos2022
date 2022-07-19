#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include "instrucciones_handler.h"
#include <semaphore.h>
#include <commons/log.h>
#include <string.h>


//---------------------------------------------------------------
// ----------------- ENUMS Y VARIABLES GLOBALES  ----------------
//---------------------------------------------------------------

typedef struct{
	int conexion_consola;
	unsigned int pid;
} relacion_consola_proceso;

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
	double estimacion_anterior;

}pcb;

typedef struct {
	t_list* instrucciones;
	unsigned int tam_proceso;
} argumentos_largo_plazo;

typedef struct {
	unsigned int tiempo_bloqueo;
	pcb * pcb_actualizado;
	double rafaga_anterior;
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
	double rafaga_real_anterior;
	unsigned int tiempo_bloqueo; //Esta sería la estructura correcta a recibir en el void* datos en el caso de que se envíe a bloquear un proceso
} bloqueo_pcb;

typedef struct {
	pcb* pcb_a_interrumpir;
	double rafaga_real_anterior;
} interrupcion_pcb;

typedef enum {
	INICIALIZAR_ESTRUCTURAS,
	SUSPENDER,
	DESTRUIR_ESTRUCTURAS
}accion_memoria;

// ---- LISTA DE ESTADOS ----//

t_log * planificador_logger;

t_list * new;
t_list * bloqueado;
t_list * ready;
t_list * running;
t_list * bloqueado_suspendido;
t_list * ready_suspendido;
t_list * exit_estado;
t_list * lista_relacion_consola_proceso;

sem_t semaforo_pid;
sem_t semaforo_pid_comparacion;
sem_t semaforo_pid_comparacion_exit;
sem_t semaforo_lista_new_add;
sem_t semaforo_lista_new_remove;
sem_t semaforo_lista_ready_add;
sem_t semaforo_lista_ready_remove;
sem_t semaforo_lista_ready_suspendido_remove;
sem_t semaforo_lista_ready_suspendido_add;
sem_t semaforo_bloqueado_suspendido;
sem_t semaforo_lista_running_remove;
sem_t semaforo_grado_multiprogramacion;
sem_t sem_sincro_running;
sem_t sem_sincro_new_ready;
sem_t sem_sincro_suspension;
sem_t sem_entrada_salida;

unsigned int pid_comparacion;
unsigned int pid_comparacion_exit;
unsigned int estimacion_inicial;
unsigned int limite_grado_multiprogramacion;
int dispatch;
int interrupt;
int conexion_memoria;
double alfa;
int tiempo_maximo_bloqueado;
char * algoritmo_planificacion;
bool creando_nuevo_proceso;




//---------------------------------------------------------------
// ----------------- DECLARACION DE FUNCIONES  ------------------
//---------------------------------------------------------------

mensaje_dispatch_posta* recibir_mensaje_dispatch();
pcb * recibir_pcb(int socket_cliente);
pcb * inicializar_pcb(t_list * lista_instrucciones, unsigned int tam_proceso);
pcb * pcb_create();
bool es_pid_a_desbloquear(void * pcb_desbloqueo);
bool es_pid_en_exit(void* rel_consola_proceso);
bool ordenar_por_estimacion_rafaga(void * unPcb, void* otroPcb);
void evaluar_desalojo(double tiempo_ejecucion_actual);
void leer_y_asignar_pcb(int socket_cliente, pcb* pcb_leido);
void inicializar_listas_procesos();
void pcb_destroy(pcb * pcb_destruir);
void relacion_consola_proceso_destroy(relacion_consola_proceso* relacion_cp);
void planificador_de_corto_plazo_sjf_running(mensaje_dispatch_posta* argumentos);
void exit_largo_plazo();
void planificador_de_corto_plazo_fifo_running(mensaje_dispatch_posta* argumentos);
void enviar_pcb(pcb* pcb_a_enviar, int socket_cliente);
void serializar_instrucciones(void* memoria_asignada, int desplazamiento, t_list* instrucciones);
void * hilo_new_ready (void* args);
void * hilo_de_largo_plazo (void * args);
void * hilo_pcb_new (void * args_p);
void * hilo_bloqueo_proceso (void* args_p);
void * hilo_de_corto_plazo_fifo_ready(void* argumentos);
void * hilo_de_corto_plazo_sjf_ready(void* argumentos);
void * cpu_dispatch_handler(void * argumentos);
void* serializar_pcb(pcb* pcb_a_enviar, int bytes);
double calcular_estimacion_rafaga(double rafaga_real_anterior, double estimacion_anterior);
void mediano_plazo_bloqueado_suspendido(unsigned int pid);
void* hilo_mediano_plazo_ready(void * argumentos);
void* hilo_contador_suspension_por_bloqueo(void* pid);

#endif /* PLANIFICADOR_H_ */
