#include "cpu.h"


int main(void) {
	pthread_t hilo_conexion_memoria;

	cpuLogger = log_create("cpuErrors.log", "cpuError_logger", 1, LOG_LEVEL_ERROR);
	cpu_info_logger = log_create("cpu_info.log", "cpu_info_logger", 1, LOG_LEVEL_INFO);

	cpu_config = config_create("cpu.config");

	ipMemoria = strdup(config_get_string_value(cpu_config,"IP_MEMORIA"));
	puertoMemoria = strdup(config_get_string_value(cpu_config,"PUERTO_MEMORIA"));

	conexion_memoria = conexion_servidor(ipMemoria, puertoMemoria);
	inicializar_hilo_conexion_memoria(&hilo_conexion_memoria);

	ipKernel = strdup(config_get_string_value(cpu_config, "IP_KERNEL"));
	puertoEscuchaDispatch = strdup(config_get_string_value(cpu_config,"PUERTO_ESCUCHA_DISPATCH"));
	puertoEscuchaInterrupt = strdup(config_get_string_value(cpu_config,"PUERTO_ESCUCHA_INTERRUPT"));

	conexionDispatch = conexion_servidor(ipKernel, puertoEscuchaDispatch);
	conexionInterrupt = conexion_servidor(ipKernel, puertoEscuchaInterrupt);

	entradasTlb = config_get_int_value(cpu_config,"ENTRADAS_TLB");
	reemplazoTlb = strdup(config_get_string_value(cpu_config,"REEMPLAZO_TLB"));
	retardoNoop = config_get_int_value(cpu_config,"RETARDO_NOOP");

	while(1){
		pcb* pcb_a_ejecutar = recibir_pcb(conexionDispatch);
		ciclo(pcb_a_ejecutar);
	}

	terminar_programa(conexion_memoria, conexionDispatch, conexionInterrupt, cpuLogger, cpu_config);
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

double mmu(unsigned int dir_logica, int numero_tabla_primer_nivel){
	//Calculamos numero de pagina y numero de entrada de la tabla de primer nivel
	double num_pagina = floor(dir_logica/tamanio_pagina);
	double entrada_primer_nivel = floor(num_pagina/entradas_por_tabla);

	//Se busca el numero de tabla de segundo nivel en memoria 1er paso
	mensaje_memoria mensaje_primera_entrada = OBTENER_TABLA_SEGUNDO_NIVEL;
	send(conexion_memoria, &mensaje_primera_entrada, sizeof(int), 0);
	send(conexion_memoria, &numero_tabla_primer_nivel, sizeof(int), 0);
	send(conexion_memoria, &entrada_primer_nivel, sizeof(double), 0);
	unsigned int numero_tabla_segundo_nivel;
	recv(conexion_memoria, &numero_tabla_segundo_nivel, sizeof(unsigned int), 0);

	double entrada_segundo_nivel = (int)num_pagina % entradas_por_tabla;

	//Acceso para conocer el marco 2do paso
	mensaje_memoria mensaje_segunda_entrada = OBTENER_NUMERO_MARCO;
	send(conexion_memoria, &mensaje_segunda_entrada, sizeof(int), 0);
	send(conexion_memoria, &numero_tabla_segundo_nivel, sizeof(unsigned int), 0);
	send(conexion_memoria, &entrada_segundo_nivel, sizeof(double), 0);
	int numero_marco;
	recv(conexion_memoria, &numero_marco, sizeof(int), 0);

	double desplazamiento = dir_logica - num_pagina * tamanio_pagina;
	double dir_fisica = (numero_marco * tamanio_pagina) + desplazamiento;

	return dir_fisica;
}

void* conexion_memoria_handler(void* argumentos){
	while(1){
		mensaje_memoria mensaje_recibido;
		recv(conexion_memoria, &mensaje_recibido, sizeof(int), 0);
		log_info(cpu_info_logger, "Recibí de memoria el mensaje: %d", mensaje_recibido);

		switch(mensaje_recibido){
			case SOLICITAR_VALORES_GLOBALES:
				recv(conexion_memoria, &tamanio_pagina, sizeof(int), 0);
				recv(conexion_memoria, &entradas_por_tabla, sizeof(int), 0);
				break;
		}
	}
}

void inicializar_hilo_conexion_memoria(pthread_t* hilo_conexion_memoria){
	pthread_create(hilo_conexion_memoria, NULL, conexion_memoria_handler, NULL);
}


