#include "memoria.h"
int main(void) {
	abrirArchivoConfifuracion();
	configurarMemoria();
	conexion = iniciar_servidor(ipMemoria, puertoEscucha);

	while(1){
		int cliente = esperar_cliente(conexion);
		if(cliente !=0) printf("cliente_kernel: %d", cliente);
	}
	  terminar_programa(conexion, loggerMemoria, memoria_config);
	return EXIT_SUCCESS;
}

void abrirArchivoConfifuracion(){
		loggerMemoria = log_create("memoria.log", "memoria.c", 1, LOG_LEVEL_DEBUG);

		puertoEscucha = strdup(config_get_string_value(memoria_config,"PUERTO_ESCUCHA"));
		tamMemoria = config_get_int_value(memoria_config,"TAMAÑO_MEMORIA");
		tamPagina = config_get_int_value(memoria_config,"TAMAÑO_PAGINA");
		paginasPorTabla = config_get_int_value(memoria_config,"PAGINAS_POR_TABLA");
		retardoMemoria = config_get_int_value(memoria_config,"RETARDO_MEMORIA");
		algoritmoReemplazo = strdup(config_get_string_value(algoritmoReemplazo,"ALGORITMO_REEMPLAZO"));
		marcosPorProceso = config_get_int_value(memoria_config,"MARCOS_POR_PROCESO");
		retardoSwap = config_get_int_value(memoria_config,"RETARDO_SWAP");
		pathSwap = strdup(config_get_string_value(algoritmoReemplazo,"PATH_SWAP"));
		tamTabla = tamMemoria / tamPagina;
	}

void configurarMemoria(){
	memoriaPrimerNivelList = list_create();
	int i = 0;
	baseMemoria = mallinfo(tamMemoria);
	while (i < paginasPorTabla){
		int j = 0;
		memoriaPrimerNivel* tabla = malloc(sizeof(memoriaPrimerNivel));
		tabla->id = i;
		tabla->memoriaSegundoNivelList = list_create();
		tabla->nroProceso = NULL;
		while (j < paginasPorTabla) {
			memoriaSegundoNivel* memoriasegunda = malloc(sizeof(memoriaSegundoNivel));
			memoriasegunda->marco = j;
			memoriasegunda->modificado = "0";
			memoriasegunda->uso = "0";
			memoriasegunda->presencia = "0";
			list_add((tabla->memoriaSegundoNivelList),memoriasegunda);
			j++;
		}
		list_add(memoriaPrimerNivelList,tabla);
		i++;
	}
}

/*void atenderMensajes(){
	while (1) {
	    int cod_op;// = recibir_int(cliente_fd);
	    switch (cod_op) {
	    case I_O:
	    case NO_OP:
	    case READ:
	      //instruccionAux.params[0] = recibir_int(cliente_fd);
	       break;
	    case WRITE:
	    case COPY:
	      //instruccionAux.params[0] = recibir_int(cliente_fd);
	      //instruccionAux.params[1] = recibir_int(cliente_fd);
	      break;
	    case EXIT:
	      break;
	    case -1:
	    	break;
	    default:
	      break;
	    }
	}

}*/
void borrar(int pagina, int marco){
	t_list_iterator *list_iterator = list_iterator_create(memoriaPrimerNivelList);
	memoriaPrimerNivel* paginaActual;
	while (list_iterator_has_next(list_iterator))
	{
		paginaActual = list_iterator_next(list_iterator);
		memoriaSegundoNivel* marcoActual;

		t_list_iterator *pag_iterator = list_iterator_create(paginaActual->memoriaSegundoNivelList);
		while (list_iterator_has_next(pag_iterator))
		{
			marcoActual = list_iterator_next(pag_iterator);
			if (marcoActual->id == marco && paginaActual->id == pagina){
				marcoActual->presencia = "0";
				log_info(loggerMemoria, "Borro el dato Correctamente");
			}
		}
	}
}
bool obtenerTablaVacia(memoriaPrimerNivel *tabla){
	return tabla->nroProceso == NULL;
}
bool obtenerPaginaVacia(memoriaSegundoNivel *pagina){
	return pagina->uso == "0";
}
void crearProceso(int procesId){
	memoriaPrimerNivel * mem =list_find(memoriaPrimerNivelList,(void*) obtenerTablaVacia);
	if(mem == NULL){
		log_error(loggerMemoria,"NO QUEDA ESPACIO PARA UN NUEVO PROCESO");
	}else{
		mem->nroProceso = procesId;
		log_info(loggerMemoria,"CREO PROCESO EXISTOSAMENTE");
	}
}

PaginaEspesifica* escribir(int dato, int pid){
	PaginaEspesifica * paginaARetonar = escirbirSinSwap(dato,pid);
	if (paginaARetonar == NULL){
		PaginaEspesifica* paginaASwappear = obtenerPaginaASwapear();
		//paginaARetonar = swappear(dato, pid,paginaASwappear);
	}
	return paginaARetonar;
}
PaginaEspesifica* escirbirSinSwap(int dato, int pid){
	memoriaPrimerNivel * mem = list_find(memoriaPrimerNivelList,(void*) obtenerTablaVacia);
	if(mem == NULL){
		//NO DEBERIA PASAR NUNCA..
		return NULL;
	}
	memoriaSegundoNivel * pag = list_find(mem->memoriaSegundoNivelList,(void*) obtenerPaginaVacia);
	if(pag == NULL){
		return NULL;
	}
	pag->uso = "1";
	pag->presencia = "1";
	pag->modificado = "1";
	memcpy(&baseMemoria + mem->id*tamTabla + tamPagina * pag->id,dato,sizeof(int));
	PaginaEspesifica* paginaEspesifica = malloc(sizeof(PaginaEspesifica));
	paginaEspesifica->idMarco = pag->marco;
	paginaEspesifica->idTabla = mem->id;
	log_info(loggerMemoria,"ESCRIBO PROCESO EXITOSAMENTE");
	return paginaEspesifica;
}
PaginaEspesifica* obtenerPaginaASwapear(){
	PaginaEspesifica * paginaARetornar = malloc(sizeof(PaginaEspesifica));
	t_list_iterator *list_iterator = list_iterator_create(memoriaPrimerNivelList);
	memoriaPrimerNivel* paginaActual;
	while (list_iterator_has_next(list_iterator))
	{
		paginaActual = list_iterator_next(list_iterator);
		memoriaSegundoNivel* marcoActual;

		t_list_iterator *pag_iterator = list_iterator_create(paginaActual->memoriaSegundoNivelList);
		while (list_iterator_has_next(pag_iterator))
		{
			marcoActual = list_iterator_next(pag_iterator);
			if(algoritmoReemplazo == "CLOCK"){
				if (marcoActual->uso == "0"){
					//marcoActual->uso = "1";
					log_info(loggerMemoria, "Encontró una pagina por clock");
					paginaARetornar->idTabla = paginaActual->id;
					paginaARetornar->idMarco = marcoActual->id;
					return paginaARetornar;
				}else{
					marcoActual->uso = "0";
				}
			}else{
				if (marcoActual->uso == "0" && marcoActual->modificado == "0"){
					//marcoActual->uso = "1";
					log_info(loggerMemoria, "Encontró una pagina por clock modificado");
					paginaARetornar->idTabla = paginaActual->id;
					paginaARetornar->idMarco = marcoActual->id;
					return paginaARetornar;
				}else{
					marcoActual->uso = "0";
					marcoActual->modificado = "0";
				}
			}
		}
	}
	return obtenerPaginaASwapear();///NO DEBERIA PASAR NUNCA PERO NECESITO UN RETURN.
}
