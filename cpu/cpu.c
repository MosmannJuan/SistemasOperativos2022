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


	while(1){
		//hacer el recieve
		sem_wait(sem_dispatch);
		PaquetePcb* paquetePcb = malloc(sizeof(PaquetePcb));
		ciclo(paquetePcb);
		sem_post(sem_dispatch);
	}

	terminar_programa(conexionMemoria, conexionDispatch, conexionInterrupt, cpuLogger, cpu_config);
}


void ciclo(PaquetePcb* paquetePcb){
	while ((paquetePcb->pcb)->pc == list_size(paquetePcb->pcb->instrucciones) || interrupciones == 1 ){
		decode_execute(paquetePcb,fetch(paquetePcb->pcb));
	}
	interrupciones = 0;
}

void* fetch(Pcb * pcb_fetch){

	return list_get(pcb_fetch->instrucciones,pcb_fetch->pc);

}

void* decode_execute (PaquetePcb * paquete_pcb_decode, Instruccion * instruccion_decode){
	paquete_pcb_decode->pcb->pc++;
	DireccionLogica * dir_logica;

	switch(instruccion_decode->tipo){
	case NO_OP:
		log_info(cpuLogger,"Ejecuto el NO_OP");
		sleep(retardoNoop);
		if(interrupciones == 1){
			paquete_pcb_decode->estado = INTERRUPCION;
			send(conexionDispatch, paquete_pcb_decode , sizeof(Pcb), 0);
		}
	break;
	case  I_O:
		log_info(cpuLogger,"Ejecuto la I_O");
		paquete_pcb_decode->estado = BLOQUEADO;
		send(conexionDispatch, paquete_pcb_decode , sizeof(Pcb), 0);
		send(conexionDispatch, &instruccion_decode->params[0],sizeof(int),0);
	break;
	case  READ:
		log_info(cpuLogger,"Ejecuto el READ");
	//	send(conexionMemoria,  )
		if(interrupciones == 1){
			paquete_pcb_decode->estado = INTERRUPCION;
					send(conexionDispatch, paquete_pcb_decode , sizeof(Pcb), 0);
				}
	break;
	case WRITE:
		log_info(cpuLogger,"Ejecuto el WRITE");
		if(interrupciones == 1){
			paquete_pcb_decode->estado = INTERRUPCION;
					send(conexionDispatch, paquete_pcb_decode , sizeof(Pcb), 0);
				}
	break;
	case  COPY:
		log_info(cpuLogger,"Ejecuto el COPY");
		dir_logica  = fetch_operands(instruccion_decode->params);
		if(interrupciones == 1){
			paquete_pcb_decode->estado = INTERRUPCION;
					send(conexionDispatch, paquete_pcb_decode , sizeof(Pcb), 0);
				}
	break;
	case  EXIT:
		log_info(cpuLogger,"Ejecuto el EXIT");
		paquete_pcb_decode->estado = FINALIZADO;
		send(conexionDispatch, paquete_pcb_decode , sizeof(Pcb), 0);
	break;
	}

return(NULL);
}

DireccionLogica* fetch_operands(unsigned int* operandos){
	DireccionLogica *direccion = malloc(sizeof(DireccionLogica));
	//obtener primer nivel a memoria, segundo nivel y desplazamiento
	return(direccion);
}
