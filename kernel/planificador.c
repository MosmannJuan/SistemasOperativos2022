#include "planificador.h"

unsigned int pid_contador = 0; //Process id

pcb inicializar_pcb(t_list * instrucciones, unsigned int tam_proceso, double estimacion_rafaga){
	pcb pcb; //= malloc(sizeof(pcb));

	pid_contador++; //TODO semaforo
	pcb.id = pid_contador;
	pcb.tam_proceso = tam_proceso;
	pcb.instrucciones = instrucciones;
	pcb.pc = 0;
	pcb.rafaga = estimacion_rafaga;
	//TODO tabla paginas

	printf("ID PROCESO: %d \n TAM PROCESO: %d \n CANTIDAD INSTRUCCIONES: %d \n PROGRAM COUNTER: %d \n ESTIMACION RAFAGA: %f \n",pcb.id, pcb.tam_proceso, list_size(pcb.instrucciones), pcb.pc, pcb.rafaga);
	return pcb;
}
