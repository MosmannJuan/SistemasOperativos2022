#include "planificador.h"

unsigned int pid_contador = 0; //Process id


// ----------------- INICIALIZACION GENERAL DE LISTAS -----------------
void inicializar_listas_procesos(){
	new = list_create();
	bloqueado = list_create();
	ready = list_create();
	running = list_create();
	bloqueado_suspendido = list_create();
	ready_suspendido = list_create();
	exit_estado = list_create();
}


// ----------------- PLANIFICADOR LARGO PLAZO -----------------

// PROCESO DE CREACION DE PCB

pcb* pcb_create(){
	pcb * pcb;
	pcb = malloc(sizeof(pcb));

	if(pcb == NULL){
		return NULL;
	}

	pcb-> instrucciones = list_create();
	if(pcb-> instrucciones  == NULL){
		free(pcb);
		return NULL;
	}

	return pcb;
}

pcb * inicializar_pcb(t_list * instrucciones, unsigned int tam_proceso, unsigned int estimacion_rafaga){
	pcb * pcb = pcb_create();

	pid_contador++; //TODO semaforo
	pcb->id = pid_contador;
	pcb->tam_proceso = tam_proceso;
	pcb->instrucciones = instrucciones;
	pcb->pc = 0;
	pcb->rafaga = estimacion_rafaga;

	printf("ID PROCESO: %d \n TAM PROCESO: %d \n CANTIDAD INSTRUCCIONES: %d \n PROGRAM COUNTER: %d \n ESTIMACION RAFAGA: %f \n",pcb->id, pcb->tam_proceso, list_size(pcb->instrucciones), pcb->pc, pcb->rafaga);
	return pcb;
}

// PROCESO DE FINALIZACION DE PROCESO

void pcb_destroy(pcb * pcb){
	list_destroy(pcb->instrucciones);
	free(pcb);
}


// FUNCION DE THREAD

void * hilo_de_largo_plazo (void * args_p){
	//Acceder a args
	argumentos_largo_plazo* pointer_args = (argumentos_largo_plazo*) args_p;
	t_list* instrucciones = pointer_args->instrucciones;
	unsigned int tam_proceso = pointer_args->tam_proceso;
	unsigned int estimacion_rafaga = pointer_args->estimacion_rafaga;
	free(args_p);
	//Inicializar pcb
	pcb * pcb_nuevo = inicializar_pcb(instrucciones, tam_proceso, estimacion_rafaga);
	//Asignar pcb a new
	printf("El tamaño de la lista de new antes de asignar es: %d \n", list_size(new));
	list_add(new, pcb_nuevo);
	printf("El tamaño de la lista de new después de asignar es: %d \n", list_size(new));
	sleep(5);
	//TODO Evaluar multiprogramacion
	//TODO Enviar mensaje a memoria

	//TODO Asignar tabla de páginas a pcb

	//Eliminar pcb de new y mover a ready
	pcb* pcb_ready = list_remove(new, 0);
	printf("El tamaño de la lista de new despues de eliminar es: %d \n", list_size(new));
	printf("El tamaño de la lista de ready antes de asignar es: %d \n", list_size(ready));
	list_add(ready, pcb_ready);
	printf("El tamaño de la lista de ready despues de asignar es: %d \n", list_size(ready));
	return NULL;

}


// ----------------- PLANIFICADOR MEDIANO PLAZO -----------------

// ----------------- PLANIFICADOR CORTO PLAZO  -----------------
