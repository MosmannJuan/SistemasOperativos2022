#include "cpu.h"


int main(void) {
	abrirArchivoConfiguracion();
	return EXIT_SUCCESS;
}


void abrirArchivoConfiguracion(){
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

void* fetch(Pcb * pcb_fetch){

	return list_get(pcb_fetch->instrucciones,pcb_fetch->pc);

}

void* decode_execute (Pcb * pcb_decode, Instruccion * instruccion_decode){
	pcb_decode->pc++;
	DireccionLogica * dir_logica;

	switch(instruccion_decode->tipo){
	case NO_OP:
		sleep(retardoNoop);
	break;
	case  I_O:
		pcb_decode->estado = BLOQUEADO;
		send(conexionDispatch, pcb_decode , sizeof(Pcb), 0);
		send(conexionDispatch, &instruccion_decode->params[0],sizeof(int),0);
	break;
	case  READ:
	//	send(conexionMemoria,  )
	break;
	case WRITE:
	break;
	case  COPY:
		dir_logica  = fetch_operands(instruccion_decode->params);
	break;
	case  EXIT:
		pcb_decode->estado = FINALIZADO;
		send(conexionDispatch, pcb_decode , sizeof(Pcb), 0);
	break;
	}

return(NULL);
}

DireccionLogica* fetch_operands(unsigned int* operandos){
	DireccionLogica *direccion = malloc(sizeof(DireccionLogica));
	//obtener primer nivel a memoria, segundo nivel y desplazamiento
	return(direccion);
}
