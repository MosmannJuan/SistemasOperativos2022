#include "kernel.h"

#include "utils.h"

int main(int argc, char ** argv) {

  t_log * loggerKernel = log_create("kernelerrors.log", "kernel.c", 1, LOG_LEVEL_ERROR);

  kernel_config = config_create("kernel.config");
  ipKernel = strdup(config_get_string_value(kernel_config, "IP_KERNEL"));
  puertoEscucha = config_get_string_value(kernel_config, "PUERTO_ESCUCHA");

  //ipMemoria = strdup(config_get_string_value(kernel_config,"IP_MEMORIA"));
  //algoritmoPlanificacion = strdup(config_get_string_value(kernel_config,"ALGORITMO_PLANIFICACION"));
  //ipCpu = strdup(config_get_string_value(kernel_config,"IP_CPU"));
  //puertoMemoria = config_get_int_value(kernel_config,"PUERTO_MEMORIA");
  //puertoCpuDispatch = config_get_int_value(kernel_config,"PUERTO_CPU_DISPATCH");
  //puertoCpuInterrupt = config_get_int_value(kernel_config,"PUERTO_CPU_INTERRUPT");
  //estimacionInicial = config_get_int_value(kernel_config,"ESTIMACION_INICIAL");
  //alfa = config_get_int_value(kernel_config,"ALFA");
  //gradoMultiprogramacion = config_get_int_value(kernel_config,"GRADO_MULTIPROGRAMACION");
  //tiempoMaximoBloqueado = config_get_int_value(kernel_config,"TIEMPO_MAXIMO_BLOQUEADO");

  int conexion = iniciar_servidor(ipKernel, puertoEscucha);
  instrucciones = list_create();

  while (1) {
	// pthread requiere que el 4to argumento (argumentos de la funcion que pasamos) sea un void*
	// entonces creamos un int*, le asignamos memoria dinámica y le guardamos el int que retorna la conexión.
	int* pointer_cliente_fd = malloc(sizeof(int));
    *pointer_cliente_fd = esperar_cliente(conexion);

    if (*pointer_cliente_fd < 0) {
      //handlear error en logger y free para evitar memory leak.
      free(pointer_cliente_fd);
      log_info(loggerKernel, "Falló conexión con el cliente.");
    } else {

    	// HANDLER DE INSTRUCCIONES DE CLIENTE MEDIANTE HILOS (KLT).
    	pthread_t handler;
    	//La funcion (3er argumento) que recibe pthread debe ser del tipo void* y los argumentos (4to argumento) deben ser void*
    	if(pthread_create(&handler, NULL, atender_instrucciones_cliente, pointer_cliente_fd) != 0) {
    		// Si el pthread_create falla, handlea el error en el logger del kernel y free para evitar memory leak.
    	    free(pointer_cliente_fd);
    		log_info(loggerKernel, "No se pudo atender al cliente por error de Kernel.");
    	}

    	/*
    	 *
    	 * HANDLER DE INSTRUCCIONES DE CLIENTE MEDIANTE CREADO DE PROCESOS HIJO
    	 *
      procesoHijo = fork();
      if (procesoHijo < 0) {
        //fallo en creación del proceso hijo
        log_info(loggerKernel, "No se pudo atender al cliente por error de Kernel.");
        return EXIT_FAILURE;
      } else if (procesoHijo == 0) {
        //CHEQUEO DE CONEXIÓN, HAY QUE REMOVER LUEGO.
        sleep(5);
        printf("\n %s %d %s %d %s", "CLIENTE NRO:", cliente_fd, "PID CLIENTE:", getpid(), "\n");
        atender_instrucciones_cliente(cliente_fd);
        //Finalizar el fork, queda en loop infinito.
        return EXIT_SUCCESS;
      }
      */

    }
  }

}

void* atender_instrucciones_cliente(void* pointer_void_cliente_fd) {
	//Recibimos el int del cliente como un void* y lo reconvertimos mediante una asignación y utilizando (int*)
	// para indicar que guarde el void* como int* en cliente.
	int* pointer_int_cliente_fd = (int*) pointer_void_cliente_fd;
	//Desreferenciamos cliente para manejar al cliente (se rompe sino, no se pq jaja)
	int cliente_fd = *pointer_int_cliente_fd;
	//liberamos la memoria reservada.
	free(pointer_void_cliente_fd);
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
