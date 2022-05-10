#include "utils.h"

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
