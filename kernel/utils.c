#include "utils.h"

int conexion_a_memoria(char * ip, char * puerto) {
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

	while(contador_de_instrucciones != cantidad_de_instrucciones){
		memcpy(memoria_asignada + desplazamiento, list_remove(instrucciones, 0), sizeof(Instruccion));
		desplazamiento  += sizeof(Instruccion);
		contador_de_instrucciones++;
	}
}

void enviar_pcb(pcb* pcb_a_enviar, int socket_cliente)
{
	int bytes = 3*sizeof(unsigned int) + sizeof(double) + list_size(pcb_a_enviar->instrucciones) * sizeof(Instruccion);
	void* a_enviar = serializar_pcb(pcb_a_enviar, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}


void terminar_programa(int conexionA, int conexionB, int conexionC, t_log * logger, t_config * config) {
  log_destroy(logger);
  config_destroy(config);
  close(conexionA);
  close(conexionB);
  close(conexionC);
}
