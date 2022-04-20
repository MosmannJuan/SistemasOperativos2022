#include "kernel.h"

#include "utils.h"

int main(int argc, char ** argv) {

  pthread_t hilo_ready;
  pthread_t hilo_running;
  pthread_t hilo_new_ready;
  pthread_t hilo_exit;
  inicializar_listas_procesos();



  t_log * loggerKernel = log_create("kernelerrors.log", "kernel.c", 1, LOG_LEVEL_ERROR);

  kernel_config = config_create("kernel.config");
  ipKernel = strdup(config_get_string_value(kernel_config, "IP_KERNEL"));
  puertoEscucha = config_get_string_value(kernel_config, "PUERTO_ESCUCHA");
  ipMemoria = strdup(config_get_string_value(kernel_config,"IP_MEMORIA"));
  puertoMemoria = config_get_string_value(kernel_config,"PUERTO_MEMORIA");
  estimacion_inicial = (unsigned int) config_get_int_value(kernel_config,"ESTIMACION_INICIAL");

  algoritmoPlanificacion = strdup(config_get_string_value(kernel_config,"ALGORITMO_PLANIFICACION"));
  //ipCpu = strdup(config_get_string_value(kernel_config,"IP_CPU"));
  //puertoCpuDispatch = config_get_int_value(kernel_config,"PUERTO_CPU_DISPATCH");
  //puertoCpuInterrupt = config_get_int_value(kernel_config,"PUERTO_CPU_INTERRUPT");
  alfa = config_get_int_value(kernel_config,"ALFA");
  //gradoMultiprogramacion = config_get_int_value(kernel_config,"GRADO_MULTIPROGRAMACION");
  tiempoMaximoBloqueado = config_get_int_value(kernel_config,"TIEMPO_MAXIMO_BLOQUEADO");

  conexion_con_memoria = conexion_a_memoria(ipMemoria, puertoMemoria);

  inicializar_planificador_corto_plazo(&hilo_ready, &hilo_running);
  inicializar_planificador_largo_plazo(&hilo_new_ready, &hilo_exit);

  int conexion = iniciar_servidor(ipKernel, puertoEscucha);

  //Inicializamos el semáforo para el process id del planificador de largo plazo
  inicializar_semaforos();

  while (1) {
	// pthread requiere que el 4to argumento (argumentos de la funcion que pasamos) sea un void*
	// entonces creamos un int*, le asignamos memoria dinámica y le guardamos el int que retorna la conexión.
	t_list * instrucciones = list_create();
	argumentos *argumentos = malloc(sizeof(argumentos));
	argumentos->instrucciones = instrucciones;
	argumentos->cliente_fd = esperar_cliente(conexion);


    if (argumentos->cliente_fd < 0) {
      //handlear error en logger y free para evitar memory leak.
      free(argumentos);
      log_info(loggerKernel, "Falló conexión con el cliente.");
    }

    else {
    	// HANDLER DE INSTRUCCIONES DE CLIENTE MEDIANTE HILOS (KLT).
    	pthread_t handler;
    	//La funcion (3er argumento) que recibe pthread debe ser del tipo void* y los argumentos (4to argumento) deben ser void*
    	if(pthread_create(&handler, NULL, atender_instrucciones_cliente,argumentos) != 0) {
    		// Si el pthread_create falla, handlea el error en el logger del kernel y free para evitar memory leak.
    	    free(argumentos);
    		log_info(loggerKernel, "No se pudo atender al cliente por error de Kernel.");
    	} else {
    		//pthread_join(handler, NULL);
    		printf("Holis");
    		//pcb* pcb = inicializar_pcb(instrucciones, (unsigned int) 8, estimacionInicial);
    		//printf("EN KERNEL PAPU \n");
    		//printf("ID PROCESO: %d \n TAM PROCESO: %d \n CANTIDAD INSTRUCCIONES: %d \n PROGRAM COUNTER: %d \n ESTIMACION RAFAGA: %f \n",pcb->id, pcb->tam_proceso, list_size(pcb->instrucciones), pcb->pc, pcb->rafaga);
    	}

    }
  }
  terminar_programa(conexion, loggerKernel, kernel_config);
}

void inicializar_semaforos(){
	sem_init(&semaforo_pid, 0, 1);
	sem_init(&semaforo_lista_new_add, 0, 1);
	sem_init(&semaforo_lista_ready_add, 0, 1);
	sem_init(&semaforo_lista_new_remove, 0, 1);
	sem_init(&semaforo_pid_comparacion, 0, 1);
    sem_init(&semaforo_lista_ready_suspendido_remove, 0,1);
    sem_init(&semaforo_lista_ready_suspendido_add, 0,1);
    sem_init(&semaforo_grado_multiprogramacion,0,1);
    sem_init(&semaforo_grado_multiprogramacion,0,1);
}

void inicializar_planificador_largo_plazo(pthread_t * hilo_new_ready, pthread_t  * hilo_exit){
	pthread_create(hilo_new_ready, NULL, hilo_new_ready, NULL);
	pthread_create(hilo_exit, NULL, exit_largo_plazo, NULL);
}

void inicializar_planificador_corto_plazo(pthread_t * hilo_ready, pthread_t * hilo_running){
	if(strcmp(algoritmoPlanificacion, "SRT") == 0) {
		pthread_create(hilo_ready, NULL, hilo_de_corto_plazo_sjf_ready, NULL);
		pthread_create(hilo_running, NULL, hilo_de_corto_plazo_sjf_running, NULL);
	} else {
		pthread_create(hilo_ready, NULL, hilo_de_corto_plazo_fifo_ready, NULL);
		pthread_create(hilo_running, NULL, hilo_de_corto_plazo_fifo_running, NULL);
	}
}

