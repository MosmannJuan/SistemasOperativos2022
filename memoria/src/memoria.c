#include "memoria.h"
int main(void) {
	pthread_t hilo_kernel_handler;

	abrirArchivoConfiguracion();
	conexion = iniciar_servidor(ip_memoria, puerto_escucha);

	logger_memoria = log_create("memoria.log", "memoria.c", 1, LOG_LEVEL_DEBUG);

	//Inicializo semáforo de e/s
	sem_init(&semaforo_entrada_salida, 0, 1);
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
					case ESCRIBIR: ;
						//Recibo la dirección física calculada por mmu y el valor a escribir
						datos_direccion direccion_escritura;
						recv(conexion_cpu, &direccion_escritura.direccion_fisica, sizeof(double), 0);
						recv(conexion_cpu, &direccion_escritura.tabla_segundo_nivel, sizeof(int), 0);
						recv(conexion_cpu, &direccion_escritura.entrada_tabla_segundo_nivel, sizeof(double), 0);
						unsigned int valor_a_escribir;
						recv(conexion_cpu, &valor_a_escribir, sizeof(unsigned int), 0);
						//Accedo a la dirección física y escribo el valor recibido
						int respuesta_escritura = ejecutar_escritura(direccion_escritura, valor_a_escribir);
						//Envío el int indicando si la escritura fue correcta
						send(conexion_cpu, &respuesta_escritura, sizeof(int), 0);
						break;
					case LEER: ;
					//Recibo la dirección física calculada por mmu
					datos_direccion direccion_lectura;
					recv(conexion_cpu, &direccion_lectura.direccion_fisica, sizeof(double), 0);
					recv(conexion_cpu, &direccion_lectura.tabla_segundo_nivel, sizeof(int), 0);
					recv(conexion_cpu, &direccion_lectura.entrada_tabla_segundo_nivel, sizeof(double), 0);
					//Accedo a la dirección física y leo el valor almacenado
					unsigned int valor_leido = ejecutar_lectura(direccion_lectura);
					//Envío el valor leído
					send(conexion_cpu, &valor_leido, sizeof(unsigned int), 0);
					break;
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
						log_info(logger_memoria, "Send enviado");
						break;
					case OBTENER_NUMERO_MARCO: ;
						unsigned int nro_tabla_segundo_nivel;
						recv(conexion_cpu, &nro_tabla_segundo_nivel, sizeof(unsigned int), 0);
						log_info(logger_memoria, "Recibí el nro_tabla_segundo_nivel: %d", nro_tabla_segundo_nivel);
						double entrada_2do_nivel;
						recv(conexion_cpu, &entrada_2do_nivel, sizeof(double), 0);
						log_info(logger_memoria, "Recibí el entrada_2do_nivel: %f", entrada_2do_nivel);
						t_list * tabla_2do_nivel = (t_list *)list_get(tablas_segundo_nivel, nro_tabla_segundo_nivel);

						entrada_segundo_nivel* entrada_seg_nivel = (entrada_segundo_nivel*)list_get(tabla_2do_nivel, (int)entrada_2do_nivel);

						if(!entrada_seg_nivel->presencia){
							unsigned int pid;
							//TODO: Enviar mensaje a cpu avisando el page fault para pedirle el pid del proceso en running (Lo necesitamos para encontrar el archivo de swap)
							int page_fault = -1;
							send(conexion_cpu, &page_fault, sizeof(int), 0);
							recv(conexion_cpu, &pid, sizeof(unsigned int), 0);
							if(list_size(listado_memoria_actual_por_proceso) < marcos_por_proceso){
								log_info(logger_memoria, "Voy a cargar una pagina nueva!");
								uint32_t marco_libre = *(uint32_t*) list_remove(marcos_disponibles, 0);
								entrada_seg_nivel->marco = marco_libre;
								entrada_seg_nivel->presencia = true;
								entrada_seg_nivel->uso = true;
								log_info(logger_memoria, "Y en el marco %d", entrada_seg_nivel->marco);
								void* pagina_swap = buscar_pagina_en_swap(entrada_seg_nivel->numero_pagina, pid);
								escribir_marco_en_memoria(marco_libre, pagina_swap);
								list_add_sorted(listado_memoria_actual_por_proceso, entrada_seg_nivel, ordenar_por_numero_marco);
							} else {
								reemplazar_pagina(entrada_seg_nivel, pid);
							}
						}
						uint32_t numero_marco = entrada_seg_nivel->marco;
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
	listado_memoria_actual_por_proceso = list_create();
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

	int pagina = 0;
	for (int i = 1; i <= cantidad_tablas_segundo_nivel; i++){
		//Creo tabla de segundo nivel
		t_list* tabla_segundo_nivel =  list_create();
		for(int j = 0; j < entradas_por_tabla; j++){
			//inicializo tabla de segundo nivel
			entrada_segundo_nivel* argumentos_tabla_segundo_nivel = malloc(sizeof(entrada_segundo_nivel));
			//inicializo bits de tabla sin marco asignado
			argumentos_tabla_segundo_nivel->presencia = false;
			argumentos_tabla_segundo_nivel->uso = false;
			argumentos_tabla_segundo_nivel->modificado = false;
			argumentos_tabla_segundo_nivel->numero_pagina = pagina;
			list_add(tabla_segundo_nivel, argumentos_tabla_segundo_nivel);
			pagina++;
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
	char* path_archivo_swap = obtener_nombre_archivo_swap(pid);
	//Creo el directorio en caso de que no exista
	if(mkdir(path_swap, 0777) == 0){
		log_info(logger_memoria, "Directorio %s creado correctamente", path_swap);
	} else if(errno == EEXIST){
		log_info(logger_memoria, "El directorio %s ya existe", path_swap);
	} else {
		log_error(logger_memoria, "Ha ocurrido un error al querer crear/validar existencia del directorio %s", path_swap);
	}

	//Creo el archivo de swap en el directorio
	FILE* archivo_swap = fopen(path_archivo_swap, "w+");
	fseek(archivo_swap, 0, SEEK_END);
	int size = ftell(archivo_swap);
	log_info(logger_memoria, "El tamaño es: %d", size);
	fseek(archivo_swap, tamanio_proceso, SEEK_SET);
	fputc('\0', archivo_swap);
	fseek(archivo_swap, 0, SEEK_SET);
	fseek(archivo_swap, 0, SEEK_END);
	int size_v2 = ftell(archivo_swap);
	log_info(logger_memoria, "El tamaño es: %d", size_v2);
	fclose(archivo_swap);

	log_info(logger_memoria, "Cerré el archivo");

//	int rta_truncate = truncate(path_archivo_swap, tamanio_proceso);
//	if(!rta_truncate)
//		printf("Error");
//	else
//		printf("Se creo el archivo swap con el tamanio: %d \n", sizeof(archivo_swap));
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

			case SUSPENDER: ;
				unsigned int pid_suspendido;
				int nro_tabla_pag;
				recv(conexion_kernel, &pid_suspendido, sizeof(unsigned int), 0);
				recv(conexion_kernel, &nro_tabla_pag, sizeof(int), 0);
				sem_wait(&semaforo_entrada_salida);
				enviar_proceso_swap(pid_suspendido, nro_tabla_pag);
				sem_post(&semaforo_entrada_salida);
				bool swap_ok = true;
				send(conexion_kernel, &swap_ok, sizeof(bool), 0);
				break;
			case DESTRUIR_ESTRUCTURAS: ;
				unsigned int pid_destruir;
				int nro_tabla_pag_destruir;
				recv(conexion_kernel, &pid_destruir, sizeof(unsigned int), 0);
				recv(conexion_kernel, &nro_tabla_pag_destruir, sizeof(int), 0);
				destruir_estructuras(pid_destruir, nro_tabla_pag_destruir);
				bool estructuras_eliminadas = true;
				send(conexion_kernel, &estructuras_eliminadas, sizeof(bool), 0);
				break;
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
	log_info(logger_memoria, "Se genera envio de tam_pag y cantidad_entradas_pagina");
	send(conexion_cpu, &tam_pagina, sizeof(int), 0);
	send(conexion_cpu, &entradas_por_tabla, sizeof(int), 0);
}

//---------------------------------------------------------------
// -------------------- LECTURA Y ESCRITURA ---------------------
//---------------------------------------------------------------

unsigned int ejecutar_lectura(datos_direccion direccion){
	unsigned int valor_leido;

	//Copio el valor leido en memoria física en la variable declarada
	memcpy(&valor_leido, base_memoria + (int) direccion.direccion_fisica, sizeof(unsigned int));

	//Actualizo el bit de uso de la página accedida
	t_list* tabla_segundo_nivel = (t_list*)list_get(tablas_segundo_nivel, direccion.tabla_segundo_nivel);
	entrada_segundo_nivel* pagina = (entrada_segundo_nivel*) list_get(tabla_segundo_nivel, direccion.entrada_tabla_segundo_nivel);
	log_info(logger_memoria, "El valor del bit de uso antes de modificar en la lectura es: %d; y el de modificado: %d", pagina->uso, pagina->modificado);
	pagina->uso = true;
	log_info(logger_memoria, "El valor del bit de uso luego de modificar en la lectura es: %d; y el de modificado: %d", pagina->uso, pagina->modificado);

	return valor_leido;
}

int ejecutar_escritura(datos_direccion direccion, unsigned int valor_escritura){
	//Copio el valor recibido en la posición correspondiente de memoria física
	memcpy(base_memoria + (int) direccion.direccion_fisica, &valor_escritura, sizeof(unsigned int));
	log_info(logger_memoria, "Escribo en la direccion %d, el valor %d", (int) direccion.direccion_fisica, valor_escritura);
	//Actualizo los bits de uso y modificado de la página accedida
	t_list* tabla_segundo_nivel = (t_list*)list_get(tablas_segundo_nivel, direccion.tabla_segundo_nivel);
	entrada_segundo_nivel* pagina = (entrada_segundo_nivel*) list_get(tabla_segundo_nivel, direccion.entrada_tabla_segundo_nivel);
	log_info(logger_memoria, "El valor del bit de uso antes de modificar en la escritura es: %d; y el de modificado: %d", pagina->uso, pagina->modificado);
	pagina->uso = true;
	pagina->modificado = true;
	log_info(logger_memoria, "El valor del bit de uso luego de modificar en la escritura es: %d; y el de modificado: %d", pagina->uso, pagina->modificado);

	return 1;

}

//---------------------------------------------------------------
// --------------------------- SWAP -----------------------------
//---------------------------------------------------------------

void enviar_proceso_swap (unsigned int pid, int nro_tabla_paginas){
	log_info(logger_memoria, "Se suspende el proceso: %d cuya tabla de primer nivel es: %d", pid, nro_tabla_paginas);

	//Aplicamos el retardo de swap solicitado
	sleep(retardo_swap/1000);

	//Obtengo la tabla de primer nivel correspondiente al proceso
	t_list* tabla_primer_nivel_proceso = (t_list*) list_get(tablas_primer_nivel, nro_tabla_paginas);

	//Obtengo la cantidad de entradas que posee la tabla (Podría no usar todo el límite asignado por archivo de config)
	int cantidad_entradas_primer_nivel = list_size(tabla_primer_nivel_proceso);

	//Recorro la tabla de primer nivel
	for(int i = 0; i < cantidad_entradas_primer_nivel; i++){
		//Obtengo el número de tabla de segundo nivel asociado a la entrada particular
		unsigned int nro_tabla_segundo_nivel = ((entrada_primer_nivel*) list_get(tabla_primer_nivel_proceso, i))->id_segundo_nivel;

		//Obtengo la tabla de segundo nivel asociada a la entrada
		t_list* tabla_segundo_nivel_entrada = (t_list*) list_get(tablas_segundo_nivel, nro_tabla_segundo_nivel);

		//Obtengo la cantidad de entradas que posee la tabla
		int cantidad_entradas_segundo_nivel = list_size(tabla_segundo_nivel_entrada);

		for(int j = 0; j < cantidad_entradas_segundo_nivel; j++){
			//Obtengo la entrada de tabla de segundo nivel
			entrada_segundo_nivel* pagina = (entrada_segundo_nivel*) list_get(tabla_segundo_nivel_entrada, j);

			//Evaluo si la pagina está modificada
			if(pagina->modificado && pagina->presencia){
				//Obtengo el nombre del archivo swap
				char* path_archivo_swap = obtener_nombre_archivo_swap(pid);
				//Leo de memoria el marco completo
				void* contenido_de_marco_leido = leer_marco_completo(pagina->marco);

				//Abro el archivo y escribo los datos
				FILE * archivo = fopen(path_archivo_swap, "r+");

				//Reubico el puntero del file en la posicion correspondiente a la página
				fseek(archivo, pagina->numero_pagina * tam_pagina, SEEK_SET);

				//Escribo el contenido del marco
				fwrite(contenido_de_marco_leido, tam_pagina, 1, archivo);
				fclose(archivo);

				//Paso presencia a 0 y libero el marco
				pagina->presencia = false;
				uint32_t* marco_disponible = malloc(sizeof(uint32_t));
				*marco_disponible = pagina->marco;
				list_add(marcos_disponibles, marco_disponible);
			}
		}

	}
}

char* obtener_nombre_archivo_swap(unsigned int pid){
	char* nombre_archivo_swap = string_itoa(pid);
	string_append(&nombre_archivo_swap, ".swap");
	char * path_archivo_swap = string_new();
	string_append(&path_archivo_swap, path_swap);
	string_append(&path_archivo_swap, "/");
	string_append(&path_archivo_swap, nombre_archivo_swap);

	return path_archivo_swap;
}

void* leer_marco_completo(uint32_t numero_marco){

	void * marco_leido = malloc(tam_pagina);

	memcpy(marco_leido, base_memoria + (int) (numero_marco * tam_pagina), tam_pagina);

	return marco_leido;
}

void* buscar_pagina_en_swap(int numero_pagina, unsigned int pid){
	//Aplicamos el retardo de swap solicitado
	sleep(retardo_swap/1000);

	char* nombre_archivo_swap = obtener_nombre_archivo_swap(pid);
	log_info(logger_memoria, "Estoy abriendo el archivo %s", nombre_archivo_swap);
	FILE* archivo_swap = fopen(nombre_archivo_swap, "r+");
	fseek(archivo_swap, numero_pagina * tam_pagina, SEEK_SET);

	void* pagina_leida = malloc(tam_pagina);

	fread(pagina_leida, tam_pagina, 1, archivo_swap);

	fclose(archivo_swap);
	log_info(logger_memoria, "Leí bien el archivo");
	return pagina_leida;
}

void escribir_marco_en_memoria(uint32_t numero_marco, void* pagina_a_escribir){
	log_info(logger_memoria, "Antes de copiar el marco");
	memcpy(base_memoria + (int) (numero_marco * tam_pagina), pagina_a_escribir, tam_pagina);
	log_info(logger_memoria, "Después de copiar el marco");
	free(pagina_a_escribir);
	log_info(logger_memoria, "Después de Free");
}

void escribir_pagina_en_swap(unsigned int numero_pagina, void* contenido_pagina_reemplazada, unsigned int pid){
	//Aplicamos el retardo de swap solicitado
	sleep(retardo_swap/1000);

	char* nombre_archivo_swap = obtener_nombre_archivo_swap(pid);

	FILE* archivo_swap = fopen(nombre_archivo_swap, "r+");
	fseek(archivo_swap, numero_pagina * tam_pagina, SEEK_SET);

	fwrite(contenido_pagina_reemplazada, tam_pagina, 1, archivo_swap);

	fclose(archivo_swap);
}

//---------------------------------------------------------------
// ----------------- DESTRUCCION ESTRUCTURAS --------------------
//---------------------------------------------------------------


void destruir_estructuras(unsigned int pid, int nro_tabla_paginas){
	log_info(logger_memoria, "Destruyendo estructuras de proceso: %d", pid);
	//Obtengo la tabla de primer nivel
	t_list* tabla_primer_nivel = list_remove(tablas_primer_nivel, nro_tabla_paginas);

	//Destruyo la tabla con sus entradas
	list_destroy_and_destroy_elements(tabla_primer_nivel, liberar_entrada_primer_nivel);

	//Borro el archivo de swap del proceso
	char* path_archivo_swap = obtener_nombre_archivo_swap(pid);
	if(remove(path_archivo_swap) == 0) log_info(logger_memoria, "Se eliminó correctamente el archivo %s", path_archivo_swap);
	else log_error(logger_memoria, "No se ha podido eliminar el archivo %s", path_archivo_swap);
}

void liberar_entrada_primer_nivel(void* entrada){
	//Casteo el void* en un entrada_primer_nivel*
	entrada_primer_nivel* entrada_a_eliminar = (entrada_primer_nivel*) entrada;

	//Busco su tabla de segundo nivel asociada
	t_list* tabla_segundo_nivel = list_remove(tablas_segundo_nivel, entrada_a_eliminar->id_segundo_nivel);

	//Destruyo la tabla de segundo nivel con sus entradas
	list_destroy_and_destroy_elements(tabla_segundo_nivel, liberar_entrada_segundo_nivel);

	free(entrada_a_eliminar);
}

void liberar_entrada_segundo_nivel(void* entrada){
	//Casteo el void* en un entrada_segundo_nivel*
	entrada_segundo_nivel* entrada_a_eliminar = (entrada_segundo_nivel*) entrada;

	//Si tiene presencia, libero el frame
	if(entrada_a_eliminar->presencia){
		uint32_t* marco_a_liberar = malloc(sizeof(uint32_t));
		*marco_a_liberar = entrada_a_eliminar->marco;
		log_info(logger_memoria, "Se agrega a la lista de marcos disponibles el marco: %d", *marco_a_liberar);
		log_info(logger_memoria, "La lista de marcos disponibles antes de agregar tiene %d registros", list_size(marcos_disponibles));
		list_add(marcos_disponibles, marco_a_liberar);
		log_info(logger_memoria, "La lista de marcos disponibles después de agregar tiene %d registros", list_size(marcos_disponibles));
	}

	//Libero la memoria asignada a la página
	free(entrada_a_eliminar);
}


//---------------------------------------------------------------
// ----------------- ALGORITMOS DE REEMPLAZO --------------------
//---------------------------------------------------------------
void inicializar_listado_memoria_actual_proceso(int tabla_primer_nivel){
	//Cargo tabla de primer nivel
	t_list* tabla_de_primer_nivel = (t_list*)list_get(tablas_primer_nivel, tabla_primer_nivel);

	//Iteramos la tabla de primer nivel para buscar paginas en uso en las tablas de segundo nivel
	list_iterate(tabla_de_primer_nivel, buscar_paginas_en_tabla_segundo_nivel);

	//TODO: Ver que hacer en caso de que un proceso ya haya estado en memoria y su cursor no deba estar en 0
	//Seteo el cursor en 0
	cursor = 0;
}

void buscar_paginas_en_tabla_segundo_nivel(void* entrada_1er_nivel){
	//Casteo el void* a una entrada de primer nivel
	entrada_primer_nivel* entrada = (entrada_primer_nivel*) entrada_1er_nivel;

	//Busco la tabla de segundo nivel asociada
	t_list* tabla_de_segundo_nivel = (t_list*)list_get(tablas_segundo_nivel, entrada->id_segundo_nivel);

	//Itero la tabla de segundo nivel para cargar en la estructura las páginas con el bit de presencia en 1
	list_iterate(tabla_de_segundo_nivel, cargar_paginas_presentes);
}

void cargar_paginas_presentes(void* entrada_2do_nivel){
	//Casteo el void* a una entrada de segundo nivel
	entrada_segundo_nivel* entrada = (entrada_segundo_nivel*) entrada_2do_nivel;

	//Si la entrada está en presencia la cargo
	if(entrada->presencia)
		list_add_sorted(listado_memoria_actual_por_proceso, entrada, ordenar_por_numero_marco);
}

bool ordenar_por_numero_marco(void * unaEntrada, void * otraEntrada){
  entrada_segundo_nivel* entradaUno = (entrada_segundo_nivel*) unaEntrada;
  entrada_segundo_nivel* entradaDos = (entrada_segundo_nivel*) otraEntrada;

  return entradaUno -> marco < entradaDos -> marco;
}

void reemplazar_pagina(entrada_segundo_nivel* pagina_a_reemplazar, unsigned int pid){
	if(strcmp(algoritmo_reemplazo, "CLOCK")){
		reemplazar_pagina_clock(pagina_a_reemplazar, pid);
	} else {
		reemplazar_pagina_clock_modificado(pagina_a_reemplazar, pid);
	}

}

void reemplazar_pagina_clock(entrada_segundo_nivel* pagina_a_reemplazar, unsigned int pid){
	//Busco la pagina a la que apunta el cursor
	entrada_segundo_nivel* entrada = (entrada_segundo_nivel*) list_get(listado_memoria_actual_por_proceso, cursor);

	//Ciclo buscando la pagina que tenga uso en 0
	while(entrada->uso){
		//Modifico el bit de uso a 0
		entrada->uso = false;

		mover_cursor();

		//Busco la siguiente pagina
		entrada = (entrada_segundo_nivel*) list_get(listado_memoria_actual_por_proceso, cursor);
	}

	reemplazar_paginas(entrada, pagina_a_reemplazar, pid);
}

void reemplazar_pagina_clock_modificado(entrada_segundo_nivel* pagina_a_reemplazar, unsigned int pid){
	//Busco la pagina a la que apunta el cursor
	entrada_segundo_nivel* entrada = clock_modificado_primer_paso();
	//Si no encontré la página ejecuto el segundo paso (Busco alguna con uso en 0 y modificado en 1)
	if(entrada == NULL) entrada = clock_modificado_segundo_paso();

	reemplazar_paginas(entrada, pagina_a_reemplazar, pid);
}

entrada_segundo_nivel* clock_modificado_primer_paso(){
	entrada_segundo_nivel* entrada;
	//Busco si algun elemento de la lista tiene bit de uso y modificado en 0
		for(int i = 0; i < list_size(listado_memoria_actual_por_proceso); i++){
			entrada  = (entrada_segundo_nivel*) list_get(listado_memoria_actual_por_proceso, cursor);
			if(!entrada->uso && !entrada->modificado){
				//Si la encuentro retorno la página encontrada
				return entrada;
			}
			//Desplazo el cursor
			mover_cursor();
		}
		//Si no la encontré en toda la vuelta retorno Null
		return NULL;
}

entrada_segundo_nivel* clock_modificado_segundo_paso(){
	//Busco la página que tenga bit de uso en 0 y modificado en 1
	entrada_segundo_nivel* entrada = buscar_pagina_modif_sin_uso();
	//Si no encontré, verifico si alguna quedó con uso y modificado en 0
	if(entrada == NULL) entrada = clock_modificado_primer_paso();
	//Si sigo sin encontrarla vuelvo a buscar alguna con uso en 1 y modificado en 1
	if(entrada == NULL) entrada = buscar_pagina_modif_sin_uso();

	return entrada;
}

entrada_segundo_nivel* buscar_pagina_modif_sin_uso(){
	entrada_segundo_nivel* entrada;
	//Busco una página que tenga bit de uso en 0 y modificado en 1
	for(int i = 0; i < list_size(listado_memoria_actual_por_proceso); i++){
		entrada  = (entrada_segundo_nivel*) list_get(listado_memoria_actual_por_proceso, cursor);
		if(!entrada->uso && entrada->modificado){
			//Si la encuentro retorno la página encontrada
			return entrada;
		}
		//Si no la encuentro cambio el bit de uso y desplazo el cursor
		entrada->uso = false;
		mover_cursor();
	}
	//Si no la encontré en toda la vuelta retorno Null
	return NULL;
}


void mover_cursor(){
	//Muevo el cursor
	cursor++;
	//Si me pase del tamaño maximo de la lista, vuelvo el cursor al inicio
	if(cursor == marcos_por_proceso) cursor = 0;
}

void reemplazar_paginas(entrada_segundo_nivel* pagina_a_swap, entrada_segundo_nivel* pagina_a_memoria, unsigned int pid){
	//Leo de swap y de memoria los contenidos de las paginas
	void* pagina_swap = buscar_pagina_en_swap(pagina_a_memoria->numero_pagina, pid);
	void* pagina_reemplazada = leer_marco_completo(pagina_a_swap->marco);

	//Asigno el marco a la pagina a reemplazar y cambio bits de presencia
	pagina_a_memoria->marco = pagina_a_swap->marco;
	pagina_a_memoria->presencia = true;
	pagina_a_swap->presencia = false;

	//Agrego la pagina reemplazada a la lista de marcos en presencia y remuevo la entrada anterior
	list_remove(listado_memoria_actual_por_proceso, cursor);
	list_add_sorted(listado_memoria_actual_por_proceso, pagina_a_memoria, ordenar_por_numero_marco);

	//Escribo en memoria la pagina traída de swap
	escribir_marco_en_memoria(pagina_a_memoria->marco, pagina_swap);

	//Escribo en swap el contenido de la pagina reemplazada si está modificada
	if(pagina_a_swap->modificado)
	escribir_pagina_en_swap(pagina_a_swap->numero_pagina, pagina_reemplazada, pid);

	//Vuelvo a mover el cursor
	mover_cursor();
}
