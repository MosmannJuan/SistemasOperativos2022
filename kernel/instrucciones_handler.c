#include "instrucciones_handler.h"

void* atender_instrucciones_cliente(void* pointer_argumentos) {
	//Recibimos el int del cliente como un void* y lo reconvertimos mediante una asignación y utilizando (int*)
	// para indicar que guarde el void* como int* en cliente.
	argumentos* pointer_args = (argumentos*) pointer_argumentos;
	int cliente_fd = *pointer_args->cliente_fd;
	t_list* instrucciones = pointer_args->instrucciones;
	free(pointer_args->cliente_fd);
	free(pointer_args);

	//sleep para testing de multiples conexiones.
    sleep(5);
    printf("\n %s %d %s %d %s %lu %s", "CLIENTE NRO:", cliente_fd, "PID CLIENTE:", getpid(), "HILO:", pthread_self(), "\n");
  Instruccion instruccionAux;
  while (1) {
    int cod_op = recibir_int(cliente_fd);
    instruccionAux.tipo = cod_op;
    switch (cod_op) {
    case I_O:
    case NO_OP:
    case READ:
      instruccionAux.params[0] = recibir_int(cliente_fd);
      printf("Recibí la instruccion %d, con el param %d \n", cod_op, instruccionAux.params[0]);
      break;
    case WRITE:
    case COPY:
      instruccionAux.params[0] = recibir_int(cliente_fd);
      instruccionAux.params[1] = recibir_int(cliente_fd);
      printf("Recibí la instruccion %d, con el params %d y %d \n", cod_op, instruccionAux.params[0], instruccionAux.params[1]);
      break;
    case EXIT:
      printf("Lei correctamente el codigo completo. \n");
      break;
    case -1:
      printf("Se ha cerrado la conexión. \n\n");
      return NULL;
    default:
      printf("No recibi un codigo de operacion valido. \n");
      break;
    }
    list_add(instrucciones, & instruccionAux);

    //int server = esperar_cliente(connection);
    //conexiones *conn = malloc(sizeof(conexiones));
    //conn->conn_kernel = server;
    //conn->ipMemoria = ip_memoria;
    //conn->puertoMemoria = puerto_memoria;
    //pthread_create(&hilo_por_cliente, NULL, (void*) atender_kernel, conn);
    //pthread_detach(hilo_por_cliente);
  }
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


