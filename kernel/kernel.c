#include "kernel.h"

#include "utils.h"

int recibir_int(int socket_cliente) {
  int leido;
  if (recv(socket_cliente, & leido, sizeof(int), MSG_WAITALL) > 0)
    return leido;
  else {
    close(socket_cliente);
    return -1;
  }
}

int atender_instrucciones_cliente(int cliente_fd, t_list * instrucciones) {
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
      printf("Se ha cerrado la conexión.");
      return EXIT_SUCCESS;
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
  t_list * instrucciones = list_create();

  while (1) {
    int cliente_fd = esperar_cliente(conexion);
    if (cliente_fd < 0) {
      //handlear error en logger
      log_info(loggerKernel, "Falló conexión con el cliente.");
    } else {
      procesoHijo = fork();
      if (procesoHijo < 0) {
          log_info(loggerKernel, "No se pudo atender al cliente por error de Kernel.");
      } else if(procesoHijo == 0) {
      	  //CHEQUEO DE CONEXIÓN, HAY QUE REMOVER LUEGO.
    	  sleep(5);
          printf("\n %s %d %s %d %s", "CLIENTE NRO:", cliente_fd, "PID CLIENTE:", getpid(), "\n");
          atender_instrucciones_cliente(cliente_fd, instrucciones);
          //Finalizar el fork, queda en loop infinito.
          return EXIT_SUCCESS;
      }
    }
  }

}
