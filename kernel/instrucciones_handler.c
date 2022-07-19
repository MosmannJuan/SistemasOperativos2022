#include "instrucciones_handler.h"

void* atender_instrucciones_cliente(void* pointer_argumentos) {
	//Recibimos el int del cliente como un void* y lo reconvertimos mediante una asignación y utilizando (int*)
	// para indicar que guarde el void* como int* en cliente.
	argumentos* pointer_args = (argumentos*) pointer_argumentos;
	int cliente_fd = pointer_args->cliente_fd;
	t_list* instrucciones = pointer_args->instrucciones;

	free(pointer_args);

	//Recibe primero el tamaño del proceso.
	unsigned int tam_proceso;
	recv(cliente_fd, &tam_proceso, sizeof(unsigned int), MSG_WAITALL);
  while (1) {
	Instruccion* instruccionAux = malloc(sizeof(Instruccion));
	instruccionAux->params[0] = 0;
	instruccionAux->params[1] = 0;
    int cod_op = recibir_int(cliente_fd);
    instruccionAux->tipo = cod_op;
    switch (cod_op) {
    case I_O:
    case READ:
      instruccionAux->params[0] = recibir_int(cliente_fd);
      break;
    case WRITE:
    case COPY:
      instruccionAux->params[0] = recibir_int(cliente_fd);
      instruccionAux->params[1] = recibir_int(cliente_fd);
      break;
    case NO_OP:
    case EXIT:
      break;
    case -1:
      iniciar_thread_largo_plazo(instrucciones, tam_proceso, cliente_fd);
      return NULL;
    default:
      break;
    }
    list_add(instrucciones, instruccionAux);
  }


}


void iniciar_thread_largo_plazo(t_list * instrucciones,  unsigned int tam_proceso, int socket_cliente){
	pthread_t largo_plazo_thread;
	argumentos_largo_plazo *args_largo_plazo = malloc(sizeof(argumentos_largo_plazo));
	args_largo_plazo->instrucciones = instrucciones;
	args_largo_plazo->tam_proceso = tam_proceso;

	pthread_create(&largo_plazo_thread, NULL, hilo_pcb_new, args_largo_plazo);
	void* retorno_hilo;
	pthread_join(largo_plazo_thread, &retorno_hilo);

//	void * retorno_hilo = hilo_pcb_new(args_largo_plazo);

	relacion_consola_proceso* rel_consola_proceso = malloc(sizeof(relacion_consola_proceso));

	rel_consola_proceso->pid = *((unsigned int*) retorno_hilo);
	free(retorno_hilo);
	rel_consola_proceso->conexion_consola = socket_cliente;

	printf("El socket: %d corresponde al proceso nro: %d", rel_consola_proceso->conexion_consola, rel_consola_proceso->pid);

	list_add(lista_relacion_consola_proceso, rel_consola_proceso);
	creando_nuevo_proceso = false;
	sem_post(&sem_sincro_new_ready);
}


int recibir_int(int socket_cliente) {
  int leido;
  recv(socket_cliente, & leido, sizeof(int), MSG_WAITALL);
  return leido;
}


