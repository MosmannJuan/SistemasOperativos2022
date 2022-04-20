#include "cpu.h"


int main(void) {
	cpuLogger = log_create("cpuErrors.log", "cpuError_logger", 1, LOG_LEVEL_ERROR);

	cpu_config = config_create("cpu.config");

	ipMemoria = strdup(config_get_string_value(cpu_config,"IP_MEMORIA"));
	puertoMemoria = strdup(config_get_string_value(cpu_config,"PUERTO_MEMORIA"));

	conexionMemoria = conexion_servidor(ipMemoria, puertoMemoria);

	ipKernel = strdup(config_get_string_value(cpu_config, "IP_KERNEL"));
	puertoEscuchaDispatch = strdup(config_get_string_value(cpu_config,"PUERTO_ESCUCHA_DISPATCH"));
	puertoEscuchaInterrupt = strdup(config_get_string_value(cpu_config,"PUERTO_ESCUCHA_INTERRUPT"));

	conexionDispatch = conexion_servidor(ipKernel, puertoEscuchaDispatch);
	conexionInterrupt = conexion_servidor(ipKernel, puertoEscuchaInterrupt);

	entradasTlb = config_get_int_value(cpu_config,"ENTRADAS_TLB");
	reemplazoTlb = strdup(config_get_string_value(cpu_config,"REEMPLAZO_TLB"));
	retardoNoop = config_get_int_value(cpu_config,"RETARDO_NOOP");


	terminar_programa(conexionMemoria, conexionDispatch, conexionInterrupt, cpuLogger, cpu_config);
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
