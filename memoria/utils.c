#include "utils.h"

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
  log_info(logger, "Se conecto un cliente a memoria!");

  return socket_cliente;
}
