#include "planificador.h"

unsigned int pid_contador = 0; //Process id


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

pcb * inicializar_pcb(t_list * instrucciones, unsigned int tam_proceso, double estimacion_rafaga){
	pcb * pcb = pcb_create();

	pid_contador++; //TODO semaforo
	pcb->id = pid_contador;
	pcb->tam_proceso = tam_proceso;
	pcb->instrucciones = instrucciones;
	pcb->pc = 0;
	pcb->rafaga = estimacion_rafaga;
	//TODO tabla paginas

	printf("ID PROCESO: %d \n TAM PROCESO: %d \n CANTIDAD INSTRUCCIONES: %d \n PROGRAM COUNTER: %d \n ESTIMACION RAFAGA: %f \n",pcb->id, pcb->tam_proceso, list_size(pcb->instrucciones), pcb->pc, pcb->rafaga);
	return pcb;
}

// PROCESO DE FINALIZACION DE PROCESO

void pcb_destroy(pcb * pcb){
	list_destroy(pcb->instrucciones);
	free(pcb);
}




// ----------------- PLANIFICADOR MEDIANO PLAZO -----------------

// ----------------- PLANIFICADOR CORTO PLAZO  -----------------
