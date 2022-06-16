#include "cpu.h"


int main(void) {
	pthread_t hilo_contador_rafaga;
	pthread_t hilo_interrupcion_handler;
	contador_rafaga_inicializado = false;

	cpu_logger = log_create("cpuErrors.log", "cpuError_logger", 1, LOG_LEVEL_ERROR);
	cpu_info_logger = log_create("cpu_info.log", "cpu_info_logger", 1, LOG_LEVEL_INFO);

	cpu_config = config_create("cpu.config");

	ipMemoria = strdup(config_get_string_value(cpu_config,"IP_MEMORIA"));
	puertoMemoria = strdup(config_get_string_value(cpu_config,"PUERTO_MEMORIA"));

	conexion_memoria = conexion_servidor(ipMemoria, puertoMemoria);
	recibir_valores_globales_memoria();

	ipKernel = strdup(config_get_string_value(cpu_config, "IP_KERNEL"));
	puertoEscuchaDispatch = strdup(config_get_string_value(cpu_config,"PUERTO_ESCUCHA_DISPATCH"));
	puertoEscuchaInterrupt = strdup(config_get_string_value(cpu_config,"PUERTO_ESCUCHA_INTERRUPT"));

	conexion_dispatch = conexion_servidor(ipKernel, puertoEscuchaDispatch);
	conexion_interrupt = conexion_servidor(ipKernel, puertoEscuchaInterrupt);

	inicializar_hilo_conexion_interrupt(&hilo_interrupcion_handler);

	entradasTlb = config_get_int_value(cpu_config,"ENTRADAS_TLB");
	reemplazoTlb = strdup(config_get_string_value(cpu_config,"REEMPLAZO_TLB"));
	retardoNoop = config_get_int_value(cpu_config,"RETARDO_NOOP");

	while(1){
		mensaje_cpu mensaje_recibido;
		recv(conexion_dispatch, &mensaje_recibido, sizeof(int), 0);
		log_info(cpu_info_logger, "Recibí de kernel el mensaje: %d", mensaje_recibido);
		switch(mensaje_recibido){
			case EVALUAR_DESALOJO:
				send(conexion_dispatch, &mensaje_recibido, sizeof(int), 0);
				send(conexion_dispatch, &contador_rafaga, sizeof(double), 0);
				break;
			case EJECUTAR:
				contador_rafaga = 0;
				hay_interrupciones = false;
				if(!contador_rafaga_inicializado) pthread_create(&hilo_contador_rafaga, NULL, contador, NULL);
				pcb* pcb_a_ejecutar = recibir_pcb(conexion_dispatch);
				ciclo(pcb_a_ejecutar);
				break;
		}
	}
	terminar_programa(conexion_memoria, conexion_dispatch, conexion_interrupt, cpu_logger, cpu_config);
}


//---------------------------------------------------------------
// ------------------ CICLO DE INSTRUCCION  ---------------------
//---------------------------------------------------------------


void ciclo(pcb* pcb_a_ejecutar){
	detener_ejecucion = false;
	while (pcb_a_ejecutar->pc <= list_size(pcb_a_ejecutar->instrucciones) && !detener_ejecucion){
		decode(pcb_a_ejecutar,fetch(pcb_a_ejecutar));
		pcb_a_ejecutar->pc++;
		if(hay_interrupciones) atender_interrupcion(pcb_a_ejecutar);
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
	case I_O:
		ejecutar_I_O(pcb_a_ejecutar, instruccion_decode->params[0]);

	break;
	case READ:
		ejecutar_READ(instruccion_decode->params[0], pcb_a_ejecutar->tabla_paginas);

	break;
	case WRITE:
		ejecutar_WRITE(instruccion_decode->params[0], instruccion_decode->params[1], pcb_a_ejecutar->tabla_paginas);
	break;
	case COPY: ;
		unsigned int operando = fetch_operands(instruccion_decode->params[0], pcb_a_ejecutar->tabla_paginas);
		ejecutar_WRITE(instruccion_decode->params[1], operando, pcb_a_ejecutar->tabla_paginas);
	break;
	case EXIT:
		ejecutar_exit(pcb_a_ejecutar);
	break;
	}

return(NULL);
}


//---------------------------------------------------------------
// --------------- EJECUCION DE INSTRUCCIONES -------------------
//---------------------------------------------------------------

void ejecutar_NO_OP(unsigned int parametro){
	log_info(cpu_logger,"Ejecuto el NO_OP");
	int contador = 0;

	while(contador != parametro){
		sleep(retardoNoop/1000);
		contador++;
	}
}

void ejecutar_I_O(pcb* pcb_a_bloquear, unsigned int tiempo_bloqueo){
	log_info(cpu_logger,"Ejecuto la I_O");
	pcb_a_bloquear->pc++;
	enviar_pcb_bloqueo(pcb_a_bloquear, tiempo_bloqueo, conexion_dispatch);
	detener_ejecucion = true;

}

void ejecutar_exit(){
	log_info(cpu_logger,"Ejecuto el EXIT");
	enviar_exit(conexion_dispatch);
	detener_ejecucion = true;
}

void ejecutar_READ(unsigned int direccion_logica, int tabla_paginas){
	//Calculo la dirección física a través de la MMU
	double direccion_fisica = mmu(direccion_logica, tabla_paginas);

	//Envío el mensaje a memoria para la lectura
	mensaje_memoria mensaje_lectura = LEER;
	send(conexion_memoria, &mensaje_lectura, sizeof(int), 0);
	send(conexion_memoria, &direccion_fisica, sizeof(double), 0);

	//Recibo el valor leído de memoria
	unsigned int valor_leido;
	recv(conexion_memoria, &valor_leido, sizeof(unsigned int), 0);
	log_info(cpu_info_logger, "El valor leído es: %d", valor_leido);
}

void ejecutar_WRITE(unsigned int direccion_logica, unsigned int valor_a_escribir, int tabla_paginas){
	//Calculo la dirección física a través de la MMU
	double direccion_fisica = mmu(direccion_logica, tabla_paginas);

	//Envío el mensaje a memoria para la lectura
	mensaje_memoria mensaje_escritura = ESCRIBIR;
	send(conexion_memoria, &mensaje_escritura, sizeof(int), 0);
	send(conexion_memoria, &direccion_fisica, sizeof(double), 0);
	send(conexion_memoria, &valor_a_escribir, sizeof(unsigned int), 0);

	//Espero el mensaje de memoria indicando que terminó la escritura OK
	int resultado_escritura;
	recv(conexion_memoria, &resultado_escritura, sizeof(int), 0);
	if(!(resultado_escritura == 1)) log_info(cpu_logger, "No se ha podido escribir el mensaje solicitado en memoria");
}

unsigned int fetch_operands(unsigned int direccion_logica, int tabla_paginas){
	double direccion_fisica = mmu(direccion_logica, tabla_paginas);

	//Envío el mensaje a memoria para la lectura
	mensaje_memoria mensaje_lectura = LEER;
	send(conexion_memoria, &mensaje_lectura, sizeof(int), 0);
	send(conexion_memoria, &direccion_fisica, sizeof(double), 0);

	//Recibo el valor leído de memoria
	unsigned int valor_leido;
	recv(conexion_memoria, &valor_leido, sizeof(unsigned int), 0);

	return valor_leido;
}

//---------------------------------------------------------------
// --------------------------- MMU ------------------------------
//---------------------------------------------------------------

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



//---------------------------------------------------------------
// ----------------- COMUNICACION CON MODULOS  ------------------
//---------------------------------------------------------------

void recibir_valores_globales_memoria(){
	recv(conexion_memoria, &tamanio_pagina, sizeof(int), 0);
	recv(conexion_memoria, &entradas_por_tabla, sizeof(int), 0);
	log_info(cpu_info_logger, "Recibí de memoria el tamaño de pagina: %d y la cantidad de entradas por tabla: %d ", tamanio_pagina, entradas_por_tabla);
}

//---------------------------------------------------------------
// --------------------------- CONTADOR   -----------------------
//---------------------------------------------------------------

void* contador(void* args){
	contador_rafaga_inicializado = true;
	while(1){
		sleep(1/1000000);
		contador_rafaga++;
	}
	return NULL;
}



//---------------------------------------------------------------
// ------------------------- INTERRUPCIONES  ---------------------
//---------------------------------------------------------------


void* interrupcion_handler(void* args){
	while(1){
		int mensaje_interrupcion;
		recv(conexion_interrupt, &mensaje_interrupcion, sizeof(int), 0);
		if(mensaje_interrupcion == 1) hay_interrupciones = true;
	}
}

void inicializar_hilo_conexion_interrupt(pthread_t* hilo_interrupcion_handler){
	pthread_create(hilo_interrupcion_handler, NULL, interrupcion_handler, NULL);
}

void atender_interrupcion(pcb* pcb_interrumpido){
	detener_ejecucion = true;
	enviar_pcb_interrupcion(pcb_interrumpido, conexion_dispatch);
}
