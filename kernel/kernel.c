#include "kernel.h"

#include "utils.h"

int main(int argc, char ** argv) {

  pthread_t hilo_ready;
  pthread_t hilo_running;
  pthread_t hilo_dispatch_handler;
  pthread_t hilo_new_ready;
  pthread_t hilo_exit;
  pthread_t hilo_mediano_plazo;
  inicializar_listas_procesos();



  t_log * logger_kernel = log_create("kernelerrors.log", "kernel.c", 1, LOG_LEVEL_ERROR);

  kernel_config = config_create("kernel.config");
  ip_kernel = strdup(config_get_string_value(kernel_config, "IP_KERNEL"));
  puerto_escucha = config_get_string_value(kernel_config, "PUERTO_ESCUCHA");

  ip_memoria = strdup(config_get_string_value(kernel_config,"IP_MEMORIA"));
  puerto_memoria = config_get_string_value(kernel_config,"PUERTO_MEMORIA");

  //ip_cpu = strdup(config_get_string_value(kernel_config,"IP_CPU"));
  puerto_cpu_dispatch = strdup(config_get_string_value(kernel_config,"PUERTO_CPU_DISPATCH"));
  puerto_cpu_interrupt = strdup(config_get_string_value(kernel_config,"PUERTO_CPU_INTERRUPT"));

  estimacion_inicial = (unsigned int) config_get_int_value(kernel_config,"ESTIMACION_INICIAL");
  algoritmo_planificacion = strdup(config_get_string_value(kernel_config,"ALGORITMO_PLANIFICACION"));
  alfa = config_get_int_value(kernel_config,"ALFA");
  limite_grado_multiprogramacion = (unsigned int) config_get_int_value(kernel_config,"GRADO_MULTIPROGRAMACION");
  tiempo_maximo_bloqueado = config_get_int_value(kernel_config,"TIEMPO_MAXIMO_BLOQUEADO");

  conexion_memoria = conexion_a_memoria(ip_memoria, puerto_memoria);
  conexion_consola = iniciar_servidor(ip_kernel, puerto_escucha);
  conexion_dispatch = iniciar_servidor(ip_kernel, puerto_cpu_dispatch);
  conexion_interrupt = iniciar_servidor(ip_kernel, puerto_cpu_interrupt);

  dispatch = esperar_cliente(conexion_dispatch);
  interrupt = esperar_cliente(conexion_interrupt);

  inicializar_planificador_corto_plazo(&hilo_ready, &hilo_running);
  inicializar_planificador_largo_plazo(&hilo_new_ready, &hilo_exit);
  inicializar_planificador_mediano_plazo(&hilo_mediano_plazo);
  inicializar_cpu_dispatch_handler(&hilo_dispatch_handler);

  //Inicializamos el semáforo para el process id del planificador de largo plazo
  inicializar_semaforos();

  while (1) {
	// pthread requiere que el 4to argumento (argumentos de la funcion que pasamos) sea un void*
	// entonces creamos un int*, le asignamos memoria dinámica y le guardamos el int que retorna la conexión.
	t_list * instrucciones = list_create();
	argumentos *argumentos = malloc(sizeof(argumentos));
	argumentos->instrucciones = instrucciones;
	argumentos->cliente_fd = esperar_cliente(conexion_consola);


    if (argumentos->cliente_fd < 0) {
      //handlear error en logger y free para evitar memory leak.
      free(argumentos);
      log_info(logger_kernel, "Falló conexión con el cliente.");
    }

    else {
    	// HANDLER DE INSTRUCCIONES DE CLIENTE MEDIANTE HILOS (KLT).
    	pthread_t handler;
    	//La funcion (3er argumento) que recibe pthread debe ser del tipo void* y los argumentos (4to argumento) deben ser void*
    	if(pthread_create(&handler, NULL, atender_instrucciones_cliente,argumentos) != 0) {
    		// Si el pthread_create falla, handlea el error en el logger del kernel y free para evitar memory leak.
    	    free(argumentos);
    		log_info(logger_kernel, "No se pudo atender al cliente por error de Kernel.");
    	}
    }
  }
  terminar_programa(conexion_consola, conexion_dispatch, conexion_interrupt, logger_kernel, kernel_config);
}

void inicializar_semaforos(){
	sem_init(&semaforo_pid, 0, 1);
	sem_init(&semaforo_pid_comparacion, 0, 1);
	sem_init(&semaforo_pid_comparacion_exit, 0, 1);
	sem_init(&semaforo_lista_new_add, 0, 1);
	sem_init(&semaforo_lista_new_remove, 0, 1);
	sem_init(&semaforo_lista_ready_add, 0, 1);
	sem_init(&semaforo_lista_ready_remove, 0, 1);
    sem_init(&semaforo_lista_ready_suspendido_remove, 0,1);
    sem_init(&semaforo_lista_ready_suspendido_add, 0,1);
    sem_init(&semaforo_lista_running_remove, 0,1);
    sem_init(&semaforo_grado_multiprogramacion,0,1);
    sem_init(&sem_sincro_running,0,0);
}

void inicializar_planificador_largo_plazo(pthread_t * hiloNewReady, pthread_t  * hilo_exit){
	pthread_create(hiloNewReady, NULL, hilo_new_ready, NULL);
	// hay que armar el msje de cpu sino rompe pthread_create(hilo_exit, NULL, exit_largo_plazo, NULL);
}

void inicializar_planificador_corto_plazo(pthread_t * hilo_ready, pthread_t * hilo_running){

	if(strcmp(algoritmo_planificacion, "SRT") == 0) {
		pthread_create(hilo_ready, NULL, hilo_de_corto_plazo_sjf_ready, NULL);
		//pthread_create(hilo_running, NULL, hilo_de_corto_plazo_sjf_running, NULL); Ya no es más hilo
	} else {
		pthread_create(hilo_ready, NULL, hilo_de_corto_plazo_fifo_ready, NULL);
		//pthread_create(hilo_running, NULL, hilo_de_corto_plazo_fifo_running, NULL); Ya no es más hilo
	}
}

void inicializar_cpu_dispatch_handler(pthread_t* hilo_dispatch_handler){
	pthread_create(hilo_dispatch_handler, NULL, cpu_dispatch_handler, NULL);
}

void inicializar_planificador_mediano_plazo(pthread_t* hilo_mediano_plazo){
	pthread_create(hilo_mediano_plazo, NULL, hilo_mediano_plazo_ready, NULL);
}

