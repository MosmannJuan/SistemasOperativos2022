#include "memoria.h"
int main(void) {
	pthread_t hilo_kernel_handler;

	abrirArchivoConfiguracion();
	conexion = iniciar_servidor(ip_memoria, puerto_escucha);
	//Carga de memoria principal
	base_memoria = malloc(tam_memoria);
	//Incializacion de listas de tablas
	inicializar_listas_procesos();
	//Inicilizacion de lista de marcos disponibles
	inicializar_marcos_disponibles();

	conexion_kernel = esperar_cliente(conexion);
	if(conexion_kernel !=0) log_info(logger_memoria, "cliente_kernel: %d", conexion_kernel);
	conexion_cpu = esperar_cliente(conexion);
	if(conexion_cpu !=0) log_info(logger_memoria, "cliente_cpu: %d", conexion_cpu);

	inicializar_kernel_handler(&hilo_kernel_handler);
	enviar_globales();

	while(1){
		//Hilo principal que corresponde con el hilo de atencion de conexion con cpu
		accion_memoria_con_cpu accion_recibida;
				log_info(logger_memoria, "Esperando recibir accion de cpu en memoria");
				recv(conexion_cpu, &accion_recibida, sizeof(int), 0);
				log_info(logger_memoria, "Recibí la accion: %d de cpu", accion_recibida);
				switch(accion_recibida){
					case OBTENER_TABLA_SEGUNDO_NIVEL: ;
						int numero_tabla_primer_nivel;
						recv(conexion_cpu, &numero_tabla_primer_nivel, sizeof(int), 0);
						log_info(logger_memoria, "Recibí el numero_tabla_primer_nivel: %d", numero_tabla_primer_nivel);
						double entrada_1er_nivel;
						recv(conexion_cpu, &entrada_1er_nivel, sizeof(double), 0);
						log_info(logger_memoria, "Recibí entrada_primer_nivel: %f", entrada_1er_nivel);
						t_list * tabla_primer_nivel = (t_list *)list_get(tablas_primer_nivel, numero_tabla_primer_nivel);
						entrada_primer_nivel* numero_tabla_segundo_nivel = (entrada_primer_nivel*)list_get(tabla_primer_nivel, (int)entrada_1er_nivel);
						send(conexion_cpu, &numero_tabla_segundo_nivel->id_segundo_nivel, sizeof(unsigned int), 0);
						break;
					case OBTENER_NUMERO_MARCO: ;
						unsigned int nro_tabla_segundo_nivel;
						recv(conexion_cpu, &nro_tabla_segundo_nivel, sizeof(unsigned int), 0);
						log_info(logger_memoria, "Recibí el nro_tabla_segundo_nivel: %d", nro_tabla_segundo_nivel);
						double entrada_2do_nivel;
						recv(conexion_cpu, &entrada_2do_nivel, sizeof(double), 0);
						log_info(logger_memoria, "Recibí el entrada_2do_nivel: %f", entrada_2do_nivel);
						t_list * tabla_2do_nivel = (t_list *)list_get(tablas_segundo_nivel, nro_tabla_segundo_nivel);
						uint32_t numero_marco = ((entrada_segundo_nivel*)list_get(tabla_2do_nivel, (int)entrada_2do_nivel))->marco;
						send(conexion_cpu, &numero_marco, sizeof(uint32_t), 0);
						break;
					default:
						//Loggear error de "Memoria no pudo interpretar el mensaje recibido"
						break;
				}
	}
	terminar_programa(conexion, logger_memoria, memoria_config);
	return EXIT_SUCCESS;
}

//---------------------------------------------------------------
// ------------------ CONFIGURACION DE MEMORIA ------------------
//---------------------------------------------------------------

void abrirArchivoConfiguracion(){
		logger_memoria = log_create("memoria.log", "memoria.c", 1, LOG_LEVEL_DEBUG);
		memoria_config = config_create("memoria.config");
		ip_memoria = strdup(config_get_string_value(memoria_config,"IP_MEMORIA"));
		puerto_escucha = strdup(config_get_string_value(memoria_config,"PUERTO_ESCUCHA"));
		tam_memoria = config_get_int_value(memoria_config,"TAM_MEMORIA");
		tam_pagina = config_get_int_value(memoria_config,"TAM_PAGINA");
		entradas_por_tabla = config_get_int_value(memoria_config,"ENTRADAS_POR_TABLA");
		retardo_memoria = config_get_int_value(memoria_config,"RETARDO_MEMORIA");
		algoritmo_reemplazo = strdup(config_get_string_value(memoria_config,"ALGORITMO_REEMPLAZO"));
		marcos_por_proceso = config_get_int_value(memoria_config,"MARCOS_POR_PROCESO");
		retardo_swap = config_get_int_value(memoria_config,"RETARDO_SWAP");
		path_swap = strdup(config_get_string_value(memoria_config,"PATH_SWAP"));
}

void inicializar_listas_procesos(){
	tablas_primer_nivel = list_create();
	tablas_segundo_nivel = list_create();
}

void inicializar_marcos_disponibles(){
	marcos_disponibles = list_create();

	int cantidad_de_marcos_disponibles = tam_memoria / tam_pagina;

	for(uint32_t i = 0; i < cantidad_de_marcos_disponibles; i++){
		uint32_t* marco = malloc(sizeof(uint32_t));
		*marco = i;
		list_add(marcos_disponibles, marco);
	}
}


//---------------------------------------------------------------
// ------------------ INICIALIZACION PROCESOS -------------------
//---------------------------------------------------------------


int inicializar_estructuras_proceso(unsigned int tamanio_proceso, unsigned int pid){

	//Creo tabla de primer nivel
	t_list* tabla_primer_nivel = list_create();
	list_add(tablas_primer_nivel, tabla_primer_nivel);

	int tabla_paginas_asignada = list_size(tablas_primer_nivel) - 1;

	int cantidad_tablas_segundo_nivel = calcular_cantidad_tablas_necesarias(tamanio_proceso);
	log_info(logger_memoria, "Se necesitan %d tablas de segundo nivel", cantidad_tablas_segundo_nivel);

	int marcos_asignados = 0;

	for (int i = 1; i <= cantidad_tablas_segundo_nivel; i++){
		//Creo tabla de segundo nivel
		t_list* tabla_segundo_nivel =  list_create();
		for(int j = 0; j < entradas_por_tabla; j++){
			//inicializo tabla de segundo nivel
			entrada_segundo_nivel* argumentos_tabla_segundo_nivel = malloc(sizeof(entrada_segundo_nivel)) ;
			if(marcos_asignados < marcos_por_proceso) {
				//Asigno marcos posibles.
				uint32_t* marco_a_asignar = list_remove(marcos_disponibles, 0);
				argumentos_tabla_segundo_nivel->marco = *marco_a_asignar;
				free(marco_a_asignar);
				marcos_asignados++;
				//inicializo bits de tabla con marcos morana.
				argumentos_tabla_segundo_nivel->presencia = true;
				argumentos_tabla_segundo_nivel->uso = true;
				argumentos_tabla_segundo_nivel->modificado=false;
			} else {
				//inicializo bits de tabla sin marcos morana.
				argumentos_tabla_segundo_nivel->presencia = false;
				argumentos_tabla_segundo_nivel->uso = false;
				argumentos_tabla_segundo_nivel->modificado=false;
			}
			list_add(tabla_segundo_nivel, argumentos_tabla_segundo_nivel);
		}

		//La agrego a la lista de tablas de segundo nivel
		list_add(tablas_segundo_nivel, tabla_segundo_nivel);

		int tabla_segundo_nivel_asignada = list_size(tablas_segundo_nivel) - 1;
		//Genero la entrada de primer nivel para conectarla con la tabla de de segundo nivel
		entrada_primer_nivel* entrada = malloc(sizeof(entrada_primer_nivel));
		entrada->id_segundo_nivel = tabla_segundo_nivel_asignada;
		list_add(tabla_primer_nivel, entrada);
	}

	crear_archivo_swap(tamanio_proceso, pid);

	return tabla_paginas_asignada;
}

// ------------------ INICIALIZACION DE SWAP ------------------

void crear_archivo_swap(unsigned int tamanio_proceso, unsigned int pid){
	char* nombre_archivo_swap = string_itoa(pid);
	string_append(&nombre_archivo_swap, ".swap");
	char * path_archivo_swap = string_new();
	string_append(&path_archivo_swap, path_swap);
	string_append(&path_archivo_swap, "/");
	string_append(&path_archivo_swap, nombre_archivo_swap);

	FILE* archivo_swap = fopen(path_archivo_swap, "rw+");

	//fclose(archivo_swap);

	int rta_truncate = truncate(path_archivo_swap, tamanio_proceso);

	if(!rta_truncate)
		printf("Error");
	else
		printf("Se creo el archivo swap con el tamanio: %d \n", sizeof(archivo_swap));
}

//---------------------------------------------------------------
// ------------------ CALCULOS AUXILIARES  ------------------
//---------------------------------------------------------------


int calcular_cantidad_tablas_necesarias(unsigned int tamanio_proceso){
	return (int) (ceil( (double)tamanio_proceso / (double)(entradas_por_tabla * tam_pagina)));
}

int calcular_marcos(){
	return tam_memoria / tam_pagina;
}

//---------------------------------------------------------------
// ----------------- COMUNICACION CON MODULOS  ------------------
//---------------------------------------------------------------

void enviar_tabla_de_paginas(int identificador_tabla_de_paginas){
	send(conexion_kernel, &identificador_tabla_de_paginas, sizeof(int), 0);
}


void* conexion_kernel_handler(void* args){
	while(1){
		accion_memoria_con_kernel accion_recibida;
		log_info(logger_memoria, "Esperando recibir accion de kernel en memoria");
		recv(conexion_kernel, &accion_recibida, sizeof(int), 0);
		log_info(logger_memoria, "Recibí la accion: %d de kernel", accion_recibida);
		switch(accion_recibida){
			case INICIALIZAR_ESTRUCTURAS: ;
				unsigned int pid;
				recv(conexion_kernel, &pid, sizeof(unsigned int), 0);
				log_info(logger_memoria, "Recibí el pid: %d", pid);
				unsigned int tam_proceso;
				recv(conexion_kernel, &tam_proceso, sizeof(unsigned int), 0);
				log_info(logger_memoria, "Recibí tamaño de proceso: %d", tam_proceso);
				int tabla_de_paginas_asignada = inicializar_estructuras_proceso(tam_proceso, pid);
				log_info(logger_memoria, "El identificador de tabla de paginas recibido es: %d", tabla_de_paginas_asignada);
				enviar_tabla_de_paginas(tabla_de_paginas_asignada);
				break;
				//TODO HACER LOS OTROS CASOS POR EJ. SUSPENDER, ELIMINAR, ETC
			default:
				//Loggear error de "Memoria no pudo interpretar el mensaje recibido"
				break;
		}
	}
}

void inicializar_kernel_handler(pthread_t *hilo_kernel_handler){
  pthread_create(hilo_kernel_handler, NULL, conexion_kernel_handler, NULL);
}

void enviar_globales(){
	accion_memoria_con_cpu accion_a_enviar = SOLICITAR_VALORES_GLOBALES;
	send(conexion_cpu, &accion_a_enviar, sizeof(int), 0);
	log_info(logger_memoria, "Envío la accion: %d a cpu", accion_a_enviar);
	log_info(logger_memoria, "Se genera envio de tam_pag y cantidad_entradas_pagina");
	send(conexion_cpu, &tam_pagina, sizeof(int), 0);
	send(conexion_cpu, &entradas_por_tabla, sizeof(int), 0);
}
