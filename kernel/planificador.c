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
  pcb * pcb;
  pcb = malloc(sizeof(pcb));

  if (pcb == NULL) {
    return NULL;
  }

  pcb -> instrucciones = list_create();
  if (pcb -> instrucciones == NULL) {
    free(pcb);
    return NULL;
  }

  return pcb;
}

pcb * inicializar_pcb(t_list * instrucciones, unsigned int tam_proceso) {
  pcb * pcb = pcb_create();
  //Enviamos la señal de wait al semáforo para bloquear el recurso
  sem_wait( & semaforo_pid);
  //Accedemos al recurso compartido y ejecutamos las instrucciones de la zona crítica
  pid_contador++;
  pcb -> id = pid_contador;
  //Enviamos la señal de post para liberar el recurso
  sem_post( & semaforo_pid);
  pcb -> tam_proceso = tam_proceso;
  pcb -> instrucciones = instrucciones;
  pcb -> pc = 0;
  pcb -> rafaga = estimacion_inicial;

  printf("ID PROCESO: %d \n TAM PROCESO: %d \n CANTIDAD INSTRUCCIONES: %d \n PROGRAM COUNTER: %d \n ESTIMACION RAFAGA: %f \n", pcb -> id, pcb -> tam_proceso, list_size(pcb -> instrucciones), pcb -> pc, pcb -> rafaga);

  return pcb;
}

// PROCESO DE FINALIZACION DE PROCESO

void pcb_destroy(pcb * pcb) {
  list_destroy(pcb -> instrucciones);
  free(pcb);
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
			  //TODO Enviar mensaje a memoria

			  //TODO Asignar tabla de páginas a pcb

			  //Eliminar pcb de new y mover a ready


			  sem_wait( & semaforo_lista_new_remove);
			  pcb * pcb_ready = list_remove(new, 0);
			  sem_post( & semaforo_lista_new_remove);

			  printf("El tamaño de la lista de new despues de eliminar es: %d \n", list_size(new));
			  printf("El tamaño de la lista de ready antes de asignar es: %d \n", list_size(ready));

			  sem_wait( & semaforo_lista_ready_add);
			  if (strcmp(algoritmoPlanificacion, "SRT") == 0) {
				list_add_sorted(ready, pcb_ready, ordenar_por_estimacion_rafaga);
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

void exit_largo_plazo(mensaje_dispatch_posta* mensaje_cpu){
		sem_wait( & semaforo_lista_running_remove);
		pcb * pcb_exit= list_remove(running, 0);
		sem_post( & semaforo_lista_running_remove);
		list_add(exit_estado, pcb_exit);

		//Eviar mensaje a memoria para hacer free
		//Memoria devuelve que fue ok

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
      list_add(running, pcb_running);
      printf("El tamaño de la lista de running despues de asignar es: %d \n", list_size(running));
    }
  }
}

void planificador_de_corto_plazo_fifo_running(mensaje_dispatch_posta* mensaje_cpu) {
    //mensaje_dispatch dummy_mensaje;
    //dummy_mensaje.mensaje = 5;
    switch (mensaje_cpu->mensaje) {
    	case PASAR_A_BLOQUEADO: ;//Para arreglar error con la declaración de datos_bloqueo
    		//Casteo los datos según lo necesario en el caso particular
			bloqueo_pcb * datos_bloqueo = (bloqueo_pcb*) mensaje_cpu->datos;
			//TODO: Ver de hacer free a este pcb.
    		pcb_destroy((pcb*)list_remove(running, 0));
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
			//TODO: ver si no debemos recibir también el tiempo transcurrido y modificar la ráfaga. SI
			pcb* pcb_interrumpido = (pcb*) mensaje_cpu->datos;
			pcb_destroy((pcb*)list_remove(running, 0));
			sem_wait( & semaforo_lista_ready_add);
			//TODO: Acá deberíamos modificar la ráfaga por lo que le queda restante? O llega de cpu?
			list_add_sorted(ready, pcb_interrumpido, ordenar_por_estimacion_rafaga);
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
    //TODO: Al final del tiempo enviamos el mensaje de interrupt a CPU por socket de interrupt
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

  return NULL;
}

void evaluar_desalojo(mensaje_dispatch_posta * mensaje_cpu){
	double rafaga_cpu_ejecutada = *(double*) mensaje_cpu->datos;

	//Saco el pcb en running y le actualizo su ráfaga
	sem_wait(&semaforo_lista_running_remove);
	pcb* pcb_running = list_remove(running, 0);
	sem_post(&semaforo_lista_running_remove);
	pcb_running->rafaga -= rafaga_cpu_ejecutada;

	//Saco el pcb de mayor prioridad de la lista de ready
	sem_wait(&semaforo_lista_ready_remove);
	pcb* pcb_mayor_prioridad_ready = list_remove(ready, 0);
	sem_post(&semaforo_lista_ready_remove);

	//Luego de evaluar dejo las listas como estaban. Ya que el planificador de corto plazo actúa en función de esto.
	//TODO:Evaluar si está bien así o si se debe extraer esta lógica
	list_add(running, pcb_running);

	sem_wait(&semaforo_lista_ready_add);
	list_add_sorted(ready, pcb_mayor_prioridad_ready, ordenar_por_estimacion_rafaga);
	sem_post(&semaforo_lista_ready_add);

	//Comparo sus ráfagas
	if(pcb_mayor_prioridad_ready->rafaga < pcb_running->rafaga){
		//Se envía mensaje de interrumpir por socket interrupt
		//TODO: Modificar en función de lo necesario para cpu
		char * mensaje_interrupt = "Interrumpir!!";
		send(interrupt, mensaje_interrupt, sizeof(mensaje_interrupt), 0);
	}
}

//---------------------------------------------------------------
// ------------ MANEJO DE COMUNICACIÓN CON CPU DISPATCH ---------
//---------------------------------------------------------------

void* cpu_dispatch_handler(void* args){
	while(1){
		mensaje_dispatch_posta* mensaje_dispatch = recibir_mensaje_dispatch();
		switch(mensaje_dispatch->mensaje){
			case PASAR_A_BLOQUEADO:
			case PASAR_A_READY:
				if(strcmp(algoritmoPlanificacion, "SRT") == 0){
					planificador_de_corto_plazo_sjf_running(mensaje_dispatch);
				}else
					planificador_de_corto_plazo_fifo_running(mensaje_dispatch);
				break;
			case EVALUAR_DESALOJO:
				evaluar_desalojo(mensaje_dispatch);
				break;
			case PASAR_A_EXIT:
				exit_largo_plazo(mensaje_dispatch);
				break;
			default:
				//Loggear error: no se ha podido interpretar el mensaje de cpu. ERROR DE KERNEL
				break;
		}
	}
}

mensaje_dispatch_posta* recibir_mensaje_dispatch(){
	//El size del struct es variable, podría cpu enviarnos primero el size?
	//Quedando de esta manera
	int size; //Almacena el tamaño en bytes que tendrá toda la estructura
	recv(dispatch, &size, sizeof(int), MSG_WAITALL);
	mensaje_dispatch_posta* mensaje_recibido = malloc(size); //Quizás habría que definir un create para esta struct
	recv(dispatch, mensaje_recibido, size, 0);
	return mensaje_recibido;
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
		Instruccion* instruccion_aux = (Instruccion *)list_remove(instrucciones, 0);
		printf("\n Instruccion a enviar: \n tipo: %d \n param1: %d \n param2:%d \n", instruccion_aux->tipo, instruccion_aux->params[0], instruccion_aux->params[1]);
		memcpy(memoria_asignada + desplazamiento, instruccion_aux, sizeof(Instruccion));
		desplazamiento  += sizeof(Instruccion);
		contador_de_instrucciones++;
	}
}

void enviar_pcb(pcb* pcb_a_enviar, int socket_cliente)
{
	int bytes = 3*sizeof(unsigned int) + sizeof(double) + list_size(pcb_a_enviar->instrucciones) * sizeof(Instruccion);
	void* a_enviar = serializar_pcb(pcb_a_enviar, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	//free(a_enviar); TODO: Ver por que rompe, posible memory leak
}
