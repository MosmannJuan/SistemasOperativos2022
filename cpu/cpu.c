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
	printf("asd0");

	while(1){
		pcb* pcb_a_ejecutar = recibir_pcb(conexionDispatch);
		ciclo(pcb_a_ejecutar);
	}

	terminar_programa(conexionMemoria, conexionDispatch, conexionInterrupt, cpuLogger, cpu_config);
}


void ciclo(pcb* pcb_a_ejecutar){
	detener_ejecucion = false;
	while (pcb_a_ejecutar->pc <= list_size(pcb_a_ejecutar->instrucciones) && !detener_ejecucion){
		decode(pcb_a_ejecutar,fetch(pcb_a_ejecutar));
		pcb_a_ejecutar->pc++;
		//Evaluar Interrupcion
	}
}

void* fetch(pcb * pcb_fetch){
	return list_get(pcb_fetch->instrucciones,pcb_fetch->pc);
}

void* decode (pcb * pcb_a_ejecutar, Instruccion * instruccion_decode){
	DireccionLogica * dir_logica;

	switch(instruccion_decode->tipo){
	case NO_OP:
		ejecutar_NO_OP(instruccion_decode->params[0]);
	break;
	case  I_O:
		ejecutar_I_O(pcb_a_ejecutar, instruccion_decode->params[0]);

	break;
//	case  READ:
//		log_info(cpuLogger,"Ejecuto el READ");
//
//	break;
//	case WRITE:
//		log_info(cpuLogger,"Ejecuto el WRITE");
//
//	break;
//	case  COPY:
//		log_info(cpuLogger,"Ejecuto el COPY");
//		dir_logica  = fetch_operands(instruccion_decode->params);
//
//	break;
	case  EXIT:
		ejecutar_exit(pcb_a_ejecutar);
	break;
	}

return(NULL);
}

DireccionLogica* fetch_operands(unsigned int* operandos){
	DireccionLogica *direccion = malloc(sizeof(DireccionLogica));
	//obtener primer nivel a memoria, segundo nivel y desplazamiento
	return(direccion);
}

void ejecutar_NO_OP(unsigned int parametro){
	log_info(cpuLogger,"Ejecuto el NO_OP");
	int contador = 0;

	while(contador != parametro){
		sleep(retardoNoop/1000);
		contador++;
	}
}

void ejecutar_I_O(pcb* pcb_a_bloquear, unsigned int tiempo_bloqueo){
	log_info(cpuLogger,"Ejecuto la I_O");
	pcb_a_bloquear->pc++;
	enviar_pcb_bloqueo(pcb_a_bloquear, tiempo_bloqueo, conexionDispatch);
	detener_ejecucion = true;

}

void ejecutar_exit(){
	log_info(cpuLogger,"Ejecuto el EXIT");
	enviar_exit(conexionDispatch);
	detener_ejecucion = true;
}


