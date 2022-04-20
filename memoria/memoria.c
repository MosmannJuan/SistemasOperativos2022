#include "memoria.h"

#include "utils.h"


int main(void) {

	t_log *loggerMemoria = log_create("memoria.log", "memoria.c", 1, LOG_LEVEL_DEBUG);

	memoria_config = config_create("memoria.config");
	ipMemoria = strdup(config_get_string_value(memoria_config, "IP_MEMORIA"));
	puertoEscucha = strdup(config_get_string_value(memoria_config,"PUERTO_ESCUCHA"));

/*

	tamMemoria = config_get_int_value(memoria_config,"TAMAÑO_MEMORIA");
	tamPagina = config_get_int_value(memoria_config,"TAMAÑO_PAGINA");
	paginasPorTabla = config_get_int_value(memoria_config,"PAGINAS_POR_TABLA");
	retardoMemoria = config_get_int_value(memoria_config,"RETARDO_MEMORIA");
	algoritmoReemplazo = strdup(config_get_string_value(algoritmoReemplazo,"ALGORITMO_REEMPLAZO"));
	marcosPorProceso = config_get_int_value(memoria_config,"MARCOS_POR_PROCESO");
	retardoSwap = config_get_int_value(memoria_config,"RETARDO_SWAP");
	pathSwap = strdup(config_get_string_value(algoritmoReemplazo,"PATH_SWAP"));
*/
	int conexion = iniciar_servidor(ipMemoria, puertoEscucha);

	while(1){
		int cliente_kernel = esperar_cliente(conexion);
		if(cliente_kernel != 0)
			printf("Se conectó alguien a memoria!");
	}
	return EXIT_SUCCESS;
}


