#include "utils.h"

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

void enviar_pcb_bloqueo(pcb* pcb_a_enviar, unsigned int tiempo_bloqueo, int socket_cliente){
	int pcb_bytes = 3*sizeof(unsigned int) + sizeof(double) + list_size(pcb_a_enviar->instrucciones) * sizeof(Instruccion);
	int bytes = pcb_bytes + sizeof(unsigned int) + sizeof(double) + sizeof(int);
	//	mensaje_dispatch* mensaje_a_enviar = malloc(bytes);
//	bloqueo_pcb* datos_mensaje = malloc(sizeof(bloqueo_pcb));
//	datos_mensaje->pcb_a_bloquear = pcb_a_enviar;
//	datos_mensaje->rafaga_real_anterior = 4; //TODO: Se hardcodea, armar contador en cpu
//	datos_mensaje->tiempo_bloqueo = tiempo_bloqueo;
//	mensaje_a_enviar->datos = datos_mensaje;
//	mensaje_a_enviar->mensaje = PASAR_A_BLOQUEADO;

	void* a_enviar = serializar_mensaje_bloqueo(pcb_a_enviar, tiempo_bloqueo, bytes);

	//Enviamos estructura de bloqueo de pcb
	send(socket_cliente, a_enviar, bytes, 0);

	//free(a_enviar); TODO: Ver por que rompe, posible memory leak
}

void* serializar_mensaje_bloqueo(pcb* pcb_a_enviar, unsigned int tiempo_bloqueo, int bytes){

	printf("Antes de malloc 1 \n");
	void* memoria_asignada = malloc(bytes);
	printf("Después de malloc 1 \n");
	int desplazamiento = 0;
	mensaje_cpu mensaje = PASAR_A_BLOQUEADO;
	double rafaga_anterior = 4.0; //Se hardcodea, necesitamos enviar el valor real de ejecucion medido

	memcpy(memoria_asignada + desplazamiento, &mensaje , sizeof(int));
	desplazamiento  += sizeof(int);
	memcpy(memoria_asignada + desplazamiento, &tiempo_bloqueo , sizeof(unsigned int));
	desplazamiento  += sizeof(unsigned int);
	memcpy(memoria_asignada + desplazamiento, &rafaga_anterior , sizeof(double));
	desplazamiento  += sizeof(double);
	serializar_pcb(pcb_a_enviar, memoria_asignada, desplazamiento);



	return memoria_asignada;
}

void* serializar_pcb(pcb* pcb_a_enviar, void* memoria_asignada, int desplazamiento){


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
		Instruccion* instruccion_aux = (Instruccion *)list_get(instrucciones, contador_de_instrucciones);
		printf("\n Instruccion a enviar: \n tipo: %d \n param1: %d \n param2:%d \n", instruccion_aux->tipo, instruccion_aux->params[0], instruccion_aux->params[1]);
		memcpy(memoria_asignada + desplazamiento, instruccion_aux, sizeof(Instruccion));
		desplazamiento  += sizeof(Instruccion);
		contador_de_instrucciones++;
	}
}

void enviar_exit(int socket_cliente){

	mensaje_cpu mensaje = PASAR_A_EXIT;
	send(socket_cliente, &mensaje, sizeof(int), 0);
}

pcb * recibir_pcb(int socket_cliente){
	pcb* pcb_leido;

	pcb_leido = pcb_create();

	leer_y_asignar_pcb(socket_cliente, pcb_leido);

	return pcb_leido;
}

void leer_y_asignar_pcb(int socket_cliente, pcb* pcb_leido){

	int cantidad_de_instrucciones;
	int contador = 0;
	Instruccion * instruccion_aux;

	//Recibo el process id
	recv(socket_cliente, &(pcb_leido->id), sizeof(unsigned int), MSG_WAITALL);
	printf("Recibiendo pcb");
	//Recibo el tamaño del proceso
	recv(socket_cliente, &(pcb_leido->tam_proceso), sizeof(unsigned int), MSG_WAITALL);

	//Recibo el program counter
	recv(socket_cliente, &(pcb_leido->pc), sizeof(unsigned int), MSG_WAITALL);

	//Recibo la estimacion de rafaga
	recv(socket_cliente, &(pcb_leido->rafaga), sizeof(double), MSG_WAITALL);

	//Recibo la cantidad de instrucciones que posee el proceso
	recv(socket_cliente, &(cantidad_de_instrucciones), sizeof(int), MSG_WAITALL);

	//Recibo las instrucciones del proceso
	while(contador < cantidad_de_instrucciones){
		instruccion_aux = malloc(sizeof(Instruccion));
		recv(socket_cliente, instruccion_aux, sizeof(Instruccion), 0);
		list_add(pcb_leido->instrucciones, instruccion_aux);
		contador++;
	}

	printf("pcb recibido: \n pid: %d \n tam_proceso: %d \n pc: %d \n rafaga: %f \n cantidad de instrucciones: %d", pcb_leido->id, pcb_leido->tam_proceso, pcb_leido->pc, pcb_leido->rafaga, list_size(pcb_leido->instrucciones));
}

int conexion_servidor(char * ip, char * puerto){
  struct addrinfo hints;
  struct addrinfo * server_info;

  memset( & hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  getaddrinfo(ip, puerto, & hints, & server_info);

  int cliente = socket(server_info -> ai_family, server_info -> ai_socktype, server_info -> ai_protocol);

  connect(cliente, server_info -> ai_addr, server_info -> ai_addrlen);

  freeaddrinfo(server_info);
  return cliente;
}

int iniciar_servidor(char * IP, char * PUERTO) {
  logger = log_create("connection.log", "connection", 1, LOG_LEVEL_INFO);
  int socket_servidor;

  struct addrinfo hints, * servinfo;

  memset( & hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  getaddrinfo(IP, PUERTO, & hints, & servinfo);

  // Creamos el socket de escucha del servidor
  socket_servidor = socket(
    servinfo -> ai_family,
    servinfo -> ai_socktype,
    servinfo -> ai_protocol
  );

  // Asociamos el socket a un puerto

  bind(socket_servidor, servinfo -> ai_addr, servinfo -> ai_addrlen);

  // Escuchamos las conexiones entrantes

  listen(socket_servidor, SOMAXCONN);

  freeaddrinfo(servinfo);
  log_trace(logger, "Listo para escuchar a mi cliente");

  return socket_servidor;
}

int esperar_cliente(int socket_servidor) {
  // Aceptamos un nuevo cliente
  int socket_cliente = accept(socket_servidor, NULL, NULL);
  log_info(logger, "Se conecto un cliente a kernel!");

  return socket_cliente;
}

void terminar_programa(int conexionA, int conexionB, int conexionC, t_log * logger, t_config * config) {
  log_destroy(logger);
  config_destroy(config);
  close(conexionA);
  close(conexionB);
  close(conexionC);
}
