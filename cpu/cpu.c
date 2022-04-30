#include <stdio.h>
#include <stdlib.h>
#include <cpu.h>


int main(void) {
	abrirArchivoConfiguracion();
	return EXIT_SUCCESS;
}


void abrirArchivoConfifuracion(){
	t_log *loggerCpu = log_create("cpu.log", "cpu.c", 1, LOG_LEVEL_DEBUG);

		entradasTlb = config_get_int_value(cpuConfig,"ENTRADAS_TLB");
		reemplazoTlb = strdup(config_get_string_value(cpuConfig,"REEMPLAZO_TLB"));
		retardoNoop = config_get_int_value(cpuConfig,"RETARDO_NOP");
		ipMemoria = strdup(config_get_string_value(cpuConfig,"IP_MEMORIA"));
		puertoMemoria = config_get_int_value(cpuConfig,"PUERTO_MEMORIA");
		puertoEscuchaDispatch = config_get_int_value(cpuConfig,"PUERTO_ESCUCHA_DISPATCH");
		puertoEscuhcaInterrupt = config_get_int_value(cpuConfig,"PUERTO_ESCUHCA_INTERRUPT");
	}

/*void ciclo(Pcb pcb){


	decodeExecute(pcb,fetch(pcb));


}*/

void* fetch( Pcb pcb){

	return list_get(pcb->instrucciones,pcb->pc);

}

void* decodeExecute ( Pcb pcb, Instruccion instruccion ){
	pcb->pc++;

	switch(instruccion->tipo){
	case NO_OP:
		sleep(retardoNoop);
	break;
	case  I_O:
		pcb->estado = BLOQUEADO
		send(conexionDispatch, &pcb , sizeof(Pcb), 0);
		send(conexionDispatch, &instruccion->params[0],sizeof(int),0);
	break;
	case  READ:
	//	send(conexionMemoria,  )
	break;
	case WRITE:
	break;
	case  COPY:
		fetchOperands(instruccion->params);
	break;
	case  EXIT:
		pcb->estado = FINALIZADO;
		send(conexionDispatch, &pcb , sizeof(Pcb), 0);
	break;
	}

return(NULL);
}

DireccionLogica	fetchOperands(int[]){

	DireccionLogica dir = malloc(sizeof(DireccionLogica));
// obtener primer nivel a memoria, segundo nivel y desplazamiento

return(dir);
}
