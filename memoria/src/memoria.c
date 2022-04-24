#include "memoria.h"
int main(void) {
	abrirArchivoConfifuracion();
	configurarMemoria();
	while(1){
	//	atenderMensajes();
	}
	return EXIT_SUCCESS;
}

void abrirArchivoConfifuracion(){
	t_log *loggerMemoria = log_create("memoria.log", "memoria.c", 1, LOG_LEVEL_DEBUG);

		puertoEscucha = strdup(config_get_string_value(memoria_config,"PUERTO_ESCUCHA"));
		tamMemoria = config_get_int_value(memoria_config,"TAMAÑO_MEMORIA");
		tamPagina = config_get_int_value(memoria_config,"TAMAÑO_PAGINA");
		paginasPorTabla = config_get_int_value(memoria_config,"PAGINAS_POR_TABLA");
		retardoMemoria = config_get_int_value(memoria_config,"RETARDO_MEMORIA");
		algoritmoReemplazo = strdup(config_get_string_value(algoritmoReemplazo,"ALGORITMO_REEMPLAZO"));
		marcosPorProceso = config_get_int_value(memoria_config,"MARCOS_POR_PROCESO");
		retardoSwap = config_get_int_value(memoria_config,"RETARDO_SWAP");
		pathSwap = strdup(config_get_string_value(algoritmoReemplazo,"PATH_SWAP"));
	}

void configurarMemoria(){
	memoriaPrimerNivelList = list_create();
	int i = 0;

	while (i < paginasPorTabla){
		int j = 0;
		memoriaPrimerNivel* tabla = malloc(sizeof(memoriaPrimerNivel));
		tabla->id = i;
		tabla->numeroPagina = malloc (sizeof(tamPagina));
		tabla->memoriaSegundoNivelList = list_create();
		while (j < paginasPorTabla) {
			memoriaSegundoNivel* memoriasegunda = malloc(sizeof(memoriaSegundoNivel));
			memoriasegunda->id = j;
			memoriasegunda->marco = j;////CHEQUEAR
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
