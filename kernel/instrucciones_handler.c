#include "instrucciones_handler.h"

void* atender_instrucciones_cliente(void* pointer_argumentos) {
	//Recibimos el int del cliente como un void* y lo reconvertimos mediante una asignación y utilizando (int*)
	// para indicar que guarde el void* como int* en cliente.
	argumentos* pointer_args = (argumentos*) pointer_argumentos;
	int cliente_fd = pointer_args->cliente_fd;
	t_list* instrucciones = pointer_args->instrucciones;

	free(pointer_args);

	//sleep para testing de multiples conexiones.
    sleep(5);
    printf("\n %s %d %s %d %s %lu %s", "CLIENTE NRO:", cliente_fd, "PID CLIENTE:", getpid(), "HILO:", pthread_self(), "\n");
  while (1) {
	Instruccion* instruccionAux = malloc(sizeof(Instruccion));
    int cod_op = recibir_int(cliente_fd);
    instruccionAux->tipo = cod_op;
    switch (cod_op) {
    case I_O:
    case NO_OP:
    case READ:
      instruccionAux->params[0] = recibir_int(cliente_fd);
      printf("Recibí la instruccion %d, con el param %d \n", cod_op, instruccionAux->params[0]);
      break;
    case WRITE:
    case COPY:
      instruccionAux->params[0] = recibir_int(cliente_fd);
      instruccionAux->params[1] = recibir_int(cliente_fd);
      printf("Recibí la instruccion %d, con el params %d y %d \n", cod_op, instruccionAux->params[0], instruccionAux->params[1]);
      break;
    case EXIT:
      printf("Lei correctamente el codigo completo. \n");
      break;
    case -1:
      printf("Se ha cerrado la conexión. \n\n");
      iniciar_thread_largo_plazo(instrucciones, (unsigned int) 8); //TODO usar el tam_proceso enviado desde consola
      return NULL;
    default:
      printf("No recibi un codigo de operacion valido. \n");
      break;
    }
    list_add(instrucciones, instruccionAux);
  }


}


void iniciar_thread_largo_plazo(t_list * instrucciones,  unsigned int tam_proceso){
	pthread_t largo_plazo_thread;
	argumentos_largo_plazo *args_largo_plazo = malloc(sizeof(argumentos_largo_plazo));
	args_largo_plazo->instrucciones = instrucciones;
	args_largo_plazo->tam_proceso = tam_proceso;

	pthread_create(&largo_plazo_thread, NULL, hilo_pcb_new, args_largo_plazo);
	pthread_join(largo_plazo_thread, NULL);
	printf("Terminó el largo plazo! \n");
}


int recibir_int(int socket_cliente) {
  int leido;
  if (recv(socket_cliente, & leido, sizeof(int), MSG_WAITALL) > 0)
    return leido;
  else {
    close(socket_cliente);
    return -1;
  }
}


