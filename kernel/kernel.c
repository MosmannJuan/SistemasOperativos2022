#include "kernel.h"

#include "utils.h"

int main(int argc, char ** argv) {

  inicializar_listas_procesos();

  t_log * loggerKernel = log_create("kernelerrors.log", "kernel.c", 1, LOG_LEVEL_ERROR);

  kernel_config = config_create("kernel.config");
  ipKernel = strdup(config_get_string_value(kernel_config, "IP_KERNEL"));
  puertoEscucha = config_get_string_value(kernel_config, "PUERTO_ESCUCHA");
  estimacionInicial = (unsigned int) config_get_int_value(kernel_config,"ESTIMACION_INICIAL");

  //ipMemoria = strdup(config_get_string_value(kernel_config,"IP_MEMORIA"));
  //algoritmoPlanificacion = strdup(config_get_string_value(kernel_config,"ALGORITMO_PLANIFICACION"));
  //ipCpu = strdup(config_get_string_value(kernel_config,"IP_CPU"));
  //puertoMemoria = config_get_int_value(kernel_config,"PUERTO_MEMORIA");
  //puertoCpuDispatch = config_get_int_value(kernel_config,"PUERTO_CPU_DISPATCH");
  //puertoCpuInterrupt = config_get_int_value(kernel_config,"PUERTO_CPU_INTERRUPT");
  //alfa = config_get_int_value(kernel_config,"ALFA");
  //gradoMultiprogramacion = config_get_int_value(kernel_config,"GRADO_MULTIPROGRAMACION");
  //tiempoMaximoBloqueado = config_get_int_value(kernel_config,"TIEMPO_MAXIMO_BLOQUEADO");

  int conexion = iniciar_servidor(ipKernel, puertoEscucha);

  //Inicializamos el semáforo para el process id del planificador de largo plazo
  sem_init(&semaforo_pid, 0, 1);

  while (1) {
	// pthread requiere que el 4to argumento (argumentos de la funcion que pasamos) sea un void*
	// entonces creamos un int*, le asignamos memoria dinámica y le guardamos el int que retorna la conexión.
	t_list * instrucciones = list_create();
	argumentos *argumentos = malloc(sizeof(argumentos));
	argumentos->instrucciones = instrucciones;
	argumentos->cliente_fd = malloc(sizeof(int));
    argumentos->estimacion_inicial = estimacionInicial;
    int socket_cliente = esperar_cliente(conexion);
    *argumentos->cliente_fd = socket_cliente;



    if (*argumentos->cliente_fd < 0) {
      //handlear error en logger y free para evitar memory leak.
      free(argumentos->cliente_fd );
      free(argumentos);
      log_info(loggerKernel, "Falló conexión con el cliente.");
    }

    else {
    	// HANDLER DE INSTRUCCIONES DE CLIENTE MEDIANTE HILOS (KLT).
    	pthread_t handler;
    	//La funcion (3er argumento) que recibe pthread debe ser del tipo void* y los argumentos (4to argumento) deben ser void*
    	if(pthread_create(&handler, NULL, atender_instrucciones_cliente,argumentos) != 0) {
    		// Si el pthread_create falla, handlea el error en el logger del kernel y free para evitar memory leak.
    	    free(argumentos->cliente_fd );
    	    free(argumentos);
    		log_info(loggerKernel, "No se pudo atender al cliente por error de Kernel.");
    	} else {
    		//pthread_join(handler, NULL);
    		printf("Holis");
    		//pcb* pcb = inicializar_pcb(instrucciones, (unsigned int) 8, estimacionInicial);
    		//printf("EN KERNEL PAPU \n");
    		//printf("ID PROCESO: %d \n TAM PROCESO: %d \n CANTIDAD INSTRUCCIONES: %d \n PROGRAM COUNTER: %d \n ESTIMACION RAFAGA: %f \n",pcb->id, pcb->tam_proceso, list_size(pcb->instrucciones), pcb->pc, pcb->rafaga);
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

