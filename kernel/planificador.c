#include "planificador.h"

unsigned int pid_contador = 0; //Process id
unsigned int grado_multiprogramacion = 0;


// ----------------- INICIALIZACION GENERAL DE LISTAS -----------------

void inicializar_listas_procesos() {
  new = list_create();
  bloqueado = list_create();
  ready = list_create();
  running = list_create();
  bloqueado_suspendido = list_create();
  ready_suspendido = list_create();
  exit_estado = list_create();
}

//-------------------------------------------------------------
// ----------------- PLANIFICADOR LARGO PLAZO -----------------
//-------------------------------------------------------------


pcb * pcb_create() {
  pcb * pcb_nuevo;
  pcb_nuevo = malloc(sizeof(pcb));

  if (pcb_nuevo == NULL) {
    return NULL;
  }

  pcb_nuevo -> instrucciones = list_create();
  if (pcb_nuevo -> instrucciones == NULL) {
    free(pcb_nuevo);
    return NULL;
  }

  return pcb_nuevo;
}

pcb * inicializar_pcb(t_list * instrucciones, unsigned int tam_proceso) {
  pcb * pcb_creado = pcb_create();
  //Enviamos la señal de wait al semáforo para bloquear el recurso
  sem_wait( & semaforo_pid);
  //Accedemos al recurso compartido y ejecutamos las instrucciones de la zona crítica
  pid_contador++;
  pcb_creado -> id = pid_contador;
  //Enviamos la señal de post para liberar el recurso
  sem_post( & semaforo_pid);
  pcb_creado -> tam_proceso = tam_proceso;
  pcb_creado -> instrucciones = instrucciones;
  pcb_creado -> pc = 0;
  pcb_creado -> tabla_paginas = 0;
  pcb_creado -> rafaga = estimacion_inicial;

  printf("ID PROCESO: %d \n TAM PROCESO: %d \n CANTIDAD INSTRUCCIONES: %d \n PROGRAM COUNTER: %d \n ESTIMACION RAFAGA: %f \n", pcb_creado -> id, pcb_creado -> tam_proceso, list_size(pcb_creado -> instrucciones), pcb_creado -> pc, pcb_creado -> rafaga);

  return pcb_creado;
}

// PROCESO DE FINALIZACION DE PROCESO

void pcb_destroy(pcb * pcb_destruir) {
  list_destroy(pcb_destruir -> instrucciones);
  free(pcb_destruir);
}

void * hilo_pcb_new(void * args_p) {
  //Acceder a args
  argumentos_largo_plazo * pointer_args = (argumentos_largo_plazo * ) args_p;
  t_list * instrucciones = pointer_args -> instrucciones;
  unsigned int tam_proceso = pointer_args -> tam_proceso;
  free(args_p);
  //Inicializar pcb
  pcb * pcb_nuevo = inicializar_pcb(instrucciones, tam_proceso);
  //Asignar pcb a new
  printf("El tamaño de la lista de new antes de asignar es: %d \n", list_size(new));

  sem_wait( & semaforo_lista_new_add);
  list_add(new, pcb_nuevo);
  sem_post( & semaforo_lista_new_add);

  printf("El tamaño de la lista de new después de asignar es: %d \n", list_size(new));
  sleep(5);

  return NULL;

}

void * hilo_new_ready(void * argumentos){
	while (1){
		  if(grado_multiprogramacion < limite_grado_multiprogramacion && list_size(ready_suspendido) == 0 && list_size(new)>0){

			  sem_wait(&semaforo_lista_new_remove);
			  pcb* pcb_new = (pcb*) list_get(new, 0);
			  sem_post(&semaforo_lista_new_remove);
			  //TODO Enviar mensaje a memoria
			  accion_memoria accion_a_ejecutar = INICIALIZAR_ESTRUCTURAS;
			  send(conexion_memoria, &accion_a_ejecutar, sizeof(int), 0);
			  send(conexion_memoria, &pcb_new->id, sizeof(unsigned int), 0);
			  send(conexion_memoria, &pcb_new->tam_proceso, sizeof(unsigned int), 0);
			  //TODO Asignar tabla de páginas a pcb
			  recv(conexion_memoria, &pcb_new->tabla_paginas, sizeof(int), 0);
			  printf("Recibí la tabla de páginas: %d \n", pcb_new->tabla_paginas);
			  //Eliminar pcb de new y mover a ready

			  sem_wait( & semaforo_lista_new_remove);
			  pcb * pcb_ready = list_remove(new, 0);
			  sem_post( & semaforo_lista_new_remove);

			  printf("El tamaño de la lista de new despues de eliminar es: %d \n", list_size(new));
			  printf("El tamaño de la lista de ready antes de asignar es: %d \n", list_size(ready));

			  sem_wait( & semaforo_lista_ready_add);
			  if (strcmp(algoritmoPlanificacion, "SRT") == 0) {
				list_add_sorted(ready, pcb_ready, ordenar_por_estimacion_rafaga);
				mensaje_cpu evaluar_desalojo = EVALUAR_DESALOJO;
				send(dispatch, &evaluar_desalojo, sizeof(int), 0);
			  } else list_add(ready, pcb_ready);
			  sem_post( & semaforo_lista_ready_add);

			  sem_wait( & semaforo_grado_multiprogramacion);
			  grado_multiprogramacion++;
			  sem_post( & semaforo_grado_multiprogramacion);


			  printf("El tamaño de la lista de ready despues de asignar es: %d \n", list_size(ready));

		  }
	}

	return NULL;
}

void exit_largo_plazo(){
		sem_wait(&sem_sincro_running);
		printf("\n exit largo plazo \n");
		printf("Lista running: %d \n", list_size(running));
		sem_wait(&semaforo_lista_running_remove);
		pcb* pcb_exit = list_remove(running, 0);
		printf("pcb exit: \n pid: %d \n tam_proceso: %d \n pc: %d \n rafaga: %f \n cantidad de instrucciones: %d \n", pcb_exit->id, pcb_exit->tam_proceso, pcb_exit->pc, pcb_exit->rafaga, list_size(pcb_exit->instrucciones));
		sem_post(&semaforo_lista_running_remove);
		printf("Agrego pcb a exit");
		list_add(exit_estado, pcb_exit);

		//Eviar mensaje a memoria para hacer free
		//Memoria devuelve que fue ok
		printf("Remuevo pcb de exit y libero la memoria");
		list_remove(exit_estado,0);
		pcb_destroy(pcb_exit);

		printf("El grado de multiprogramación es: %d \n", grado_multiprogramacion);

		sem_wait( & semaforo_grado_multiprogramacion);
		grado_multiprogramacion--;
		sem_post( & semaforo_grado_multiprogramacion);

		printf("El grado de multiprogramación es: %d \n", grado_multiprogramacion);
}



//---------------------------------------------------------------
// ----------------- PLANIFICADOR MEDIANO PLAZO -----------------
//---------------------------------------------------------------


void * hilo_mediano_plazo_ready(void * argumentos) {

    printf("El grado de multiprogramación es: %d \n", grado_multiprogramacion);

  while (1) {
    if (list_size(ready_suspendido) > 0 && grado_multiprogramacion < limite_grado_multiprogramacion) {
      sem_wait( & semaforo_lista_ready_suspendido_remove);
      pcb * pcb_ready = list_remove(ready_suspendido, 0);
      sem_post( & semaforo_lista_ready_suspendido_remove);

      printf("El tamaño de la lista de new despues de eliminar es: %d \n", list_size(ready_suspendido));
      printf("El tamaño de la lista de ready antes de asignar es: %d \n", list_size(ready));
      //TODO Enviar mensaje a Memoria para volver a asignar lugar

      sem_wait( & semaforo_lista_ready_add);
      if (strcmp(algoritmoPlanificacion, "SRT") == 0) {
        list_add_sorted(ready, pcb_ready, ordenar_por_estimacion_rafaga);
      } else {
        list_add(ready, pcb_ready);
      }
      sem_post( & semaforo_lista_ready_add);
      //Envío mensaje a cpu para que el planificador evalúe el desalojo
	  mensaje_cpu evaluar_desalojo = EVALUAR_DESALOJO;
	  send(dispatch, &evaluar_desalojo, sizeof(int), 0);

      printf("El tamaño de la lista de ready despues de asignar es: %d \n", list_size(ready));

      sem_wait( & semaforo_grado_multiprogramacion);
      grado_multiprogramacion++;
      sem_post( & semaforo_grado_multiprogramacion);

      printf("El grado de multiprogramación es: %d \n", grado_multiprogramacion);

      return NULL;
    }
  }
}

void * mediano_plazo_bloqueado_suspendido(pcb * pcb_actualizado, unsigned int tiempo_bloqueo) {
  // Removemos de bloqueado y simulamos la espera del I/O
  sem_wait( & semaforo_pid_comparacion);
  pid_comparacion = pcb_actualizado -> id;
  list_remove_by_condition(bloqueado, es_pid_a_desbloquear);
  sem_post( & semaforo_pid_comparacion);

  list_add(bloqueado_suspendido, pcb_actualizado);

  //Se envia a memoria para que pase a disco

  sem_wait( & semaforo_grado_multiprogramacion);
  grado_multiprogramacion--;
  sem_post( & semaforo_grado_multiprogramacion);

  sleep(tiempo_bloqueo / 1000);

  //Saca el PCB actual de la lista de suspendido bloqueado.
  sem_wait( & semaforo_pid_comparacion);
  pid_comparacion = pcb_actualizado -> id;
  list_remove_by_condition(bloqueado_suspendido, es_pid_a_desbloquear);
  sem_post( & semaforo_pid_comparacion);
  sem_wait(&semaforo_lista_ready_suspendido_add);
  if(strcmp(algoritmoPlanificacion, "SRT") == 0)
	  list_add_sorted(ready_suspendido, pcb_actualizado, ordenar_por_estimacion_rafaga);
  else
	  list_add(ready_suspendido, pcb_actualizado);
  sem_post(&semaforo_lista_ready_suspendido_add);
  //Al terminar una entrada salida se envía un mensaje de interrupción
  int mensaje_interrupt = 1;
  send(interrupt, mensaje_interrupt, sizeof(int), 0);

  return NULL;
}




//--------------------------------------------------------------
// ----------------- PLANIFICADOR CORTO PLAZO  -----------------
//--------------------------------------------------------------


void * hilo_de_corto_plazo_fifo_ready(void * argumentos) {
  while (1) {
    if (list_size(ready) > 0 && list_size(running) == 0) {
      //Sacamos de lista de ready
      printf("El tamaño de la lista de ready antes de eliminar es: %d \n", list_size(ready));
      pcb * pcb_running = list_remove(ready, 0);
      printf("El tamaño de la lista de ready después de eliminar es: %d \n", list_size(ready));
      //TODO: Mandamos mensaje a CPU
      enviar_pcb(pcb_running, dispatch);
      //Enviamos a running
      printf("El tamaño de la lista de running antes de asignar es: %d \n", list_size(running));
      sem_wait(&semaforo_lista_running_remove);
      list_add(running, pcb_running);
      sem_post(&semaforo_lista_running_remove);
      sem_post(&sem_sincro_running);
      printf("El tamaño de la lista de running despues de asignar es: %d \n", list_size(running));
    }
  }
}

void planificador_de_corto_plazo_fifo_running(mensaje_dispatch_posta* mensaje_cpu) {
    //mensaje_dispatch dummy_mensaje;
    //dummy_mensaje.mensaje = 5;
	printf("Mensaje recibido: %d \n", mensaje_cpu->mensaje);
    switch (mensaje_cpu->mensaje) {
    	case PASAR_A_BLOQUEADO: ;//Para arreglar error con la declaración de datos_bloqueo
    		//Casteo los datos según lo necesario en el caso particular
    		printf("Pasar a bloqueado \n");
			bloqueo_pcb * datos_bloqueo = (bloqueo_pcb*) mensaje_cpu->datos;
			printf("Casteo no crasheó \n");
			printf("pcb a bloquear: \n pid: %d \n tam_proceso: %d \n pc: %d \n rafaga: %f \n cantidad de instrucciones: %d \n", datos_bloqueo->pcb_a_bloquear->id, datos_bloqueo->pcb_a_bloquear->tam_proceso, datos_bloqueo->pcb_a_bloquear->pc, datos_bloqueo->pcb_a_bloquear->rafaga, list_size(datos_bloqueo->pcb_a_bloquear->instrucciones));
			//TODO: Ver de hacer free a este pcb.
			sem_wait(&sem_sincro_running);
			pcb* pcb_destruir = (pcb*)list_remove(running, 0);
			printf("\npcb a destruir: \n pid: %d \n tam_proceso: %d \n pc: %d \n rafaga: %f \n cantidad de instrucciones: %d \n", pcb_destruir->id, pcb_destruir->tam_proceso, pcb_destruir->pc, pcb_destruir->rafaga, list_size(pcb_destruir->instrucciones));
    		pcb_destroy(pcb_destruir);
    		printf("Llegó hasta acá \n");
    		list_add(bloqueado, datos_bloqueo->pcb_a_bloquear);
    		argumentos_hilo_bloqueo * args_bloqueo = malloc(sizeof(argumentos_hilo_bloqueo));
    		args_bloqueo -> tiempo_bloqueo = datos_bloqueo->tiempo_bloqueo;
    		args_bloqueo -> pcb_actualizado = datos_bloqueo->pcb_a_bloquear;
    		pthread_t hilo_bloqueo;
    		pthread_create( & hilo_bloqueo, NULL, hilo_bloqueo_proceso, args_bloqueo);
    		break;
    	default:
    		//TODO: Loggear error de no se ha podido interpretar el mensaje de cpu
    		break;
  }
}

void * hilo_de_corto_plazo_sjf_ready(void * argumentos) {

  while (1) {
    if (list_size(ready) > 0 && list_size(running) == 0) {
      //Sacamos de lista de ready
      printf("El tamaño de la lista de ready antes de eliminar es: %d \n", list_size(ready));
      pcb * pcb_running = list_remove(ready, 0);
      printf("El tamaño de la lista de ready después de eliminar es: %d \n", list_size(ready));
      //TODO: Mandamos mensaje a CPU
      enviar_pcb(pcb_running, dispatch);
      //Enviamos a running
      printf("El tamaño de la lista de running antes de asignar es: %d \n", list_size(running));
      list_add(running, pcb_running);
      printf("El tamaño de la lista de running despues de asignar es: %d \n", list_size(running));
    }
  }
}

void planificador_de_corto_plazo_sjf_running(mensaje_dispatch_posta * mensaje_cpu) {
  //unsigned int real_anterior = 3; //ESTO VIENE CALCULADO DE CPU
    switch (mensaje_cpu->mensaje) {
		case PASAR_A_BLOQUEADO: ;//Para arreglar error con la declaración de datos_bloqueo
			//Casteo los datos según lo necesario en el caso particular
			bloqueo_pcb * datos_bloqueo = (bloqueo_pcb*) mensaje_cpu->datos;
			//TODO: Deberíamos hacer el free a este pcb?? Evaluar posible memory leak
			pcb_destroy((pcb*)list_remove(running, 0));
			datos_bloqueo->pcb_a_bloquear->rafaga = calcular_estimacion_rafaga(datos_bloqueo->rafaga_real_anterior, datos_bloqueo->pcb_a_bloquear->rafaga);
			list_add(bloqueado, datos_bloqueo->pcb_a_bloquear);
			argumentos_hilo_bloqueo * args_bloqueo = malloc(sizeof(argumentos_hilo_bloqueo));
			args_bloqueo -> tiempo_bloqueo = datos_bloqueo->tiempo_bloqueo;
			args_bloqueo -> pcb_actualizado = datos_bloqueo->pcb_a_bloquear;
			pthread_t hilo_bloqueo;
			pthread_create( & hilo_bloqueo, NULL, hilo_bloqueo_proceso, args_bloqueo);
			break;
		case PASAR_A_READY: ;//Para arreglar error con la declaración de datos_bloqueo
			//Casteo los datos según lo necesario en el caso particular
		    interrupcion_pcb* datos_interrupcion = (interrupcion_pcb*) mensaje_cpu->datos;
			pcb_destroy((pcb*)list_remove(running, 0));
			pcb* pcb_interrupcion = datos_interrupcion->pcb_a_interrumpir;
			sem_wait( & semaforo_lista_ready_add);
			//TODO: Acá deberíamos modificar la ráfaga por lo que le queda restante? O llega de cpu?
			pcb_interrupcion->rafaga -= datos_interrupcion->rafaga_real_anterior;
			list_add_sorted(ready, pcb_interrupcion, ordenar_por_estimacion_rafaga);
			sem_post( & semaforo_lista_ready_add);
			break;
		default:
			break;
  }
}


bool es_pid_a_desbloquear(void * pcb_recibido) {
  pcb * pcb_comparacion = (pcb * ) pcb_recibido;
  return pcb_comparacion -> id == pid_comparacion;
}

bool ordenar_por_estimacion_rafaga(void * unPcb, void * otroPcb) {
  pcb * pcbUno = (pcb * ) unPcb;
  pcb * pcbDos = (pcb * ) otroPcb;

  return pcbUno -> rafaga < pcbDos -> rafaga;
}

unsigned int calcular_estimacion_rafaga(unsigned int rafaga_real_anterior, unsigned int estimacion_anterior) {
	return alfa * rafaga_real_anterior + (1 - alfa) * estimacion_anterior;
}

void * hilo_bloqueo_proceso(void * argumentos) {
	printf("Hilo bloqueo \n");
  argumentos_hilo_bloqueo * args = (argumentos_hilo_bloqueo * ) argumentos;
  unsigned int tiempo_bloqueo = args -> tiempo_bloqueo;
  pcb * pcb_actualizado = args -> pcb_actualizado;
  free(args);

  //Esperamos el tiempo que corresponde según la instrucción
  unsigned int tiempo_extra = tiempo_bloqueo - tiempoMaximoBloqueado;

  if (tiempo_bloqueo > tiempoMaximoBloqueado){
	sleep(tiempoMaximoBloqueado / 1000);
    mediano_plazo_bloqueado_suspendido(pcb_actualizado, tiempo_extra);
  }
  else {
	sleep(tiempo_bloqueo / 1000);

    //Enviamos de bloqueado a ready
    sem_wait( & semaforo_pid_comparacion);
    pid_comparacion = pcb_actualizado -> id;
    list_remove_by_condition(bloqueado, es_pid_a_desbloquear);
    sem_post( & semaforo_pid_comparacion);

    sem_wait( & semaforo_lista_ready_add);

    if(strcmp(algoritmoPlanificacion, "SRT") == 0)
    	list_add_sorted(ready, pcb_actualizado, ordenar_por_estimacion_rafaga);
    else
    	list_add(ready, pcb_actualizado);

    sem_post( & semaforo_lista_ready_add);
  }

  int mensaje_interrupt = 1;
  send(interrupt, mensaje_interrupt, sizeof(int), 0);


  return NULL;
}

void evaluar_desalojo(double rafaga_cpu_ejecutada){

	//Saco el pcb en running y le actualizo su ráfaga
	sem_wait(&semaforo_lista_running_remove);
	pcb* pcb_running = list_get(running, 0);
	sem_post(&semaforo_lista_running_remove);
	pcb_running->rafaga -= rafaga_cpu_ejecutada;

	//Saco el pcb de mayor prioridad de la lista de ready
	sem_wait(&semaforo_lista_ready_remove);
	pcb* pcb_mayor_prioridad_ready = list_get(ready, 0);
	sem_post(&semaforo_lista_ready_remove);

	//Comparo sus ráfagas
	if(pcb_mayor_prioridad_ready->rafaga < pcb_running->rafaga){
		//Se envía mensaje de interrumpir por socket interrupt
		//TODO: Modificar en función de lo necesario para cpu
		int mensaje_interrupt = 1;
		send(interrupt, mensaje_interrupt, sizeof(int), 0);
	}
}

//---------------------------------------------------------------
// ------------ MANEJO DE COMUNICACIÓN CON CPU DISPATCH ---------
//---------------------------------------------------------------

void* cpu_dispatch_handler(void* args){
	while(1){
		mensaje_cpu accion_recibida;
		recv(dispatch, &accion_recibida, sizeof(int), MSG_WAITALL);
		switch(accion_recibida){
			case PASAR_A_BLOQUEADO: ;
				printf("Pasar a bloqueado \n");
				mensaje_dispatch_posta* mensaje_bloqueo = malloc(sizeof(mensaje_dispatch_posta));
				bloqueo_pcb* datos_bloqueo = malloc(sizeof(bloqueo_pcb));
				mensaje_bloqueo->datos = datos_bloqueo;
				mensaje_bloqueo->mensaje = accion_recibida;
				recv(dispatch, &(datos_bloqueo->tiempo_bloqueo), sizeof(unsigned int), 0);
				printf("Tiempo bloqueo recibido: %d", datos_bloqueo->tiempo_bloqueo);
				recv(dispatch, &(datos_bloqueo->rafaga_real_anterior), sizeof(double), 0);
				printf("rafaga_real_anterior recibida: %f", datos_bloqueo->rafaga_real_anterior);
				datos_bloqueo->pcb_a_bloquear = recibir_pcb(dispatch);
				printf("pcb a bloquear recién recibido: \n pid: %d \n tam_proceso: %d \n pc: %d \n rafaga: %f \n cantidad de instrucciones: %d \n", datos_bloqueo->pcb_a_bloquear->id, datos_bloqueo->pcb_a_bloquear->tam_proceso, datos_bloqueo->pcb_a_bloquear->pc, datos_bloqueo->pcb_a_bloquear->rafaga, list_size(datos_bloqueo->pcb_a_bloquear->instrucciones));
				if(strcmp(algoritmoPlanificacion, "SRT") == 0){
					planificador_de_corto_plazo_sjf_running(mensaje_bloqueo);
				}else{
					planificador_de_corto_plazo_fifo_running(mensaje_bloqueo);
				}
				break;
			case PASAR_A_READY: ;
				mensaje_dispatch_posta* mensaje_interrupcion = malloc(sizeof(mensaje_dispatch_posta));
				interrupcion_pcb* datos_interrupcion = malloc(sizeof(interrupcion_pcb));
				mensaje_interrupcion->datos = datos_interrupcion;
				mensaje_bloqueo->mensaje = accion_recibida;
				recv(dispatch, &datos_interrupcion->rafaga_real_anterior, sizeof(double), 0);
				datos_interrupcion->pcb_a_interrumpir = recibir_pcb(dispatch);
				if(strcmp(algoritmoPlanificacion, "SRT") == 0){
					planificador_de_corto_plazo_sjf_running(mensaje_interrupcion);
				}else
					planificador_de_corto_plazo_fifo_running(mensaje_interrupcion);
				break;
			case EVALUAR_DESALOJO: ;
				double tiempo_ejecucion_actual;
				recv(dispatch, &tiempo_ejecucion_actual, sizeof(double), 0);
				evaluar_desalojo(tiempo_ejecucion_actual);
				break;
			case PASAR_A_EXIT:
				printf("Pasar a exit");
				exit_largo_plazo();
				break;
			default:
				//Loggear error: no se ha podido interpretar el mensaje de cpu. ERROR DE KERNEL
				break;
		}
	}
	return NULL;
}

//---------------------------------------------------------------
// ----------------- SERIALIZACION Y ENVIO DE PCB ---------------
//---------------------------------------------------------------

void* serializar_pcb(pcb* pcb_a_enviar, int bytes){

	void* memoria_asignada = malloc(bytes);
	int desplazamiento = 0;

	memcpy(memoria_asignada + desplazamiento, &(pcb_a_enviar->id), sizeof(unsigned int));
	desplazamiento  += sizeof(unsigned int);
	memcpy(memoria_asignada + desplazamiento, &(pcb_a_enviar->tam_proceso), sizeof(unsigned int));
	desplazamiento  += sizeof(unsigned int);
	memcpy(memoria_asignada + desplazamiento, &(pcb_a_enviar->pc), sizeof(unsigned int));
	desplazamiento  += sizeof(unsigned int);
	memcpy(memoria_asignada + desplazamiento, &(pcb_a_enviar->rafaga), sizeof(double));
	desplazamiento  += sizeof(double);
	memcpy(memoria_asignada + desplazamiento, &(pcb_a_enviar->tabla_paginas), sizeof(int));
	desplazamiento  += sizeof(int);

	serializar_instrucciones(memoria_asignada, desplazamiento, pcb_a_enviar->instrucciones);
	return memoria_asignada;
}

void serializar_instrucciones(void* memoria_asignada, int desplazamiento, t_list* instrucciones){
	int contador_de_instrucciones = 0;
	int cantidad_de_instrucciones = list_size(instrucciones);

	//Indicamos la cantidad de instrucciones que debe leer cpu
	memcpy(memoria_asignada + desplazamiento, &(cantidad_de_instrucciones), sizeof(int));
	desplazamiento  += sizeof(int);

	while(contador_de_instrucciones < cantidad_de_instrucciones){
		Instruccion* instruccion_aux = (Instruccion *)list_get(instrucciones, contador_de_instrucciones);
		printf("\n Instruccion a enviar: \n tipo: %d \n param1: %d \n param2:%d \n", instruccion_aux->tipo, instruccion_aux->params[0], instruccion_aux->params[1]);
		memcpy(memoria_asignada + desplazamiento, instruccion_aux, sizeof(Instruccion));
		desplazamiento  += sizeof(Instruccion);
		contador_de_instrucciones++;
	}
}

void enviar_pcb(pcb* pcb_a_enviar, int socket_cliente)
{
	int bytes = 2*sizeof(int) + 3*sizeof(unsigned int) + sizeof(double) + list_size(pcb_a_enviar->instrucciones) * sizeof(Instruccion);
	void* a_enviar = serializar_pcb(pcb_a_enviar, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	//free(a_enviar); TODO: Ver por que rompe, posible memory leak
}

pcb* recibir_pcb(int socket_cliente){

	pcb* pcb_leido = pcb_create();

	leer_y_asignar_pcb(socket_cliente, pcb_leido);

	return pcb_leido;
}

void leer_y_asignar_pcb(int socket_cliente, pcb* pcb_leido){

	int cantidad_de_instrucciones;
	int contador = 0;
	Instruccion * instruccion_aux;
	//t_list * lista_aux = list_create();
	printf("pcb antes de asignar: \n pid: %d \n tam_proceso: %d \n pc: %d \n rafaga: %f \n cantidad de instrucciones: %d", pcb_leido->id, pcb_leido->tam_proceso, pcb_leido->pc, pcb_leido->rafaga, list_size(pcb_leido->instrucciones));
	//Recibo el process id
	recv(socket_cliente, &(pcb_leido->id), sizeof(unsigned int), MSG_WAITALL);
	printf("Recibiendo pcb \n");
	printf("pid: %d \n", pcb_leido->id);
	//Recibo el tamaño del proceso
	recv(socket_cliente, &(pcb_leido->tam_proceso), sizeof(unsigned int), MSG_WAITALL);
	printf("tam_proceso: %d \n", pcb_leido->tam_proceso);
	//Recibo el program counter
	recv(socket_cliente, &(pcb_leido->pc), sizeof(unsigned int), MSG_WAITALL);
	printf("pc: %d \n", pcb_leido->pc);
	//Recibo la estimacion de rafaga
	recv(socket_cliente, &(pcb_leido->rafaga), sizeof(double), MSG_WAITALL);
	printf("rafaga: %f \n", pcb_leido->rafaga);
	//Recibo la tabla de poaginas
	recv(socket_cliente, &(pcb_leido->tabla_paginas), sizeof(int), MSG_WAITALL);
	printf("tabla de paginas: %d \n", pcb_leido->tabla_paginas);
	//Recibo la cantidad de instrucciones que posee el proceso
	recv(socket_cliente, &(cantidad_de_instrucciones), sizeof(int), MSG_WAITALL);
	printf("cant instr: %d \n", cantidad_de_instrucciones);
	//Recibo las instrucciones del proceso
	while(contador < cantidad_de_instrucciones){
		instruccion_aux = malloc(sizeof(Instruccion));
		recv(socket_cliente, instruccion_aux, sizeof(Instruccion), 0);
		printf("List size %d \n", list_size(pcb_leido->instrucciones));
		list_add(pcb_leido->instrucciones, instruccion_aux);
		contador++;
	}
	//pcb_leido->instrucciones = lista_aux;

	printf("pcb recibido: \n pid: %d \n tam_proceso: %d \n pc: %d \n rafaga: %f \n tabla de paginas: %d \n cantidad de instrucciones: %d", pcb_leido->id, pcb_leido->tam_proceso, pcb_leido->pc, pcb_leido->rafaga, pcb_leido->tabla_paginas, list_size(pcb_leido->instrucciones));
}
