#include "conexiones.h"

//////////////////////////
//CONEXIONES DE SERVIDOR//
//////////////////////////

int iniciar_servidor(char* ip, char* puerto)
{
	t_log *logger = log_create("conexiones.log", "conexiones.c", 1, LOG_LEVEL_TRACE);
    struct addrinfo hints, *servinfo = NULL;

    servinfo = setDireccion(&hints, ip, puerto, servinfo);

    int socket_servidor = crearSocket(servinfo);

    iniciarEscucha(socket_servidor, servinfo);

    freeaddrinfo(servinfo);
    log_trace(logger, "Pude hacer el Bindeo y el Listen, estoy listo para escuchar clientes");
    log_destroy(logger);
    return socket_servidor;
}

void iniciarEscucha(int ficheroSocketOriginal, struct addrinfo *direccPersonal) {
	int retornoBind = bind(ficheroSocketOriginal, direccPersonal->ai_addr, direccPersonal->ai_addrlen);
	comprobarErrores(retornoBind, "Bind", true);

	int retornoListen = listen(ficheroSocketOriginal, SOMAXCONN);
	comprobarErrores(retornoListen, "Listen", true);
}

int esperar_cliente(int socket_servidor)
{
	t_log *logger = log_create("conexiones.log", "conexiones.c", 1, LOG_LEVEL_INFO);
	int socket_cliente =  aceptarConexion(socket_servidor);
	log_info(logger, "Se conecto un cliente!");
	log_destroy(logger);
	return socket_cliente;
}

int aceptarConexion(int ficheroSocketOriginal) {
	int ficheroSocketNuevo = accept(ficheroSocketOriginal, NULL, NULL);
	comprobarErrores(ficheroSocketNuevo, "Accept", true);

	return ficheroSocketNuevo;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) != 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

/////////////////////////
//CONEXIONES DE CLIENTE//
/////////////////////////

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info = NULL;

	server_info = setDireccion(&hints, ip, puerto, server_info);

	int socket_cliente = crearSocket(server_info);

	int salida = conectar(socket_cliente, server_info);

	freeaddrinfo(server_info);

	if(salida != -1)
		return socket_cliente;
	else
		return -1;
}

int conectar(int ficheroSocket, struct addrinfo *server_info) {
	int respuesta = connect(ficheroSocket,server_info->ai_addr,server_info->ai_addrlen);
	comprobarErrores(respuesta, "Connect", false);
	return respuesta;
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);
	send(socket_cliente, a_enviar, bytes, MSG_NOSIGNAL);
	free(a_enviar);
}

void cerrarConn(int socket_cliente)
{
	t_paquete* paquete = crear_paquete(END);
	crear_buffer(paquete);
	agregar_a_paquete(paquete, "", strlen("") + 1);
	enviar_paquete(paquete,socket_cliente);
	eliminar_paquete(paquete);
	uint32_t handshake;
	recv(socket_cliente, &handshake, sizeof(uint32_t), MSG_WAITALL);
}

///////////////////////
//FUNCIONES GENERALES//
///////////////////////

struct addrinfo* setDireccion(struct addrinfo* hints, char* ip, char* puerto, struct addrinfo* server_info) {
	memset(&*hints, 0, sizeof(*hints));
	hints->ai_family = AF_UNSPEC;
	hints->ai_socktype = SOCK_STREAM;
	hints->ai_flags = AI_PASSIVE;
	getaddrinfo(ip, puerto, &*hints, &server_info);
	return server_info;
}

int crearSocket(struct addrinfo *servinfo) {

	int yes = 1;

	int ficheroSocketOriginal = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	comprobarErrores(ficheroSocketOriginal, "Socket", true);

	int respuestaSetSocket = setsockopt(ficheroSocketOriginal, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	comprobarErrores(respuestaSetSocket, "Set Socket Options", true);

	return ficheroSocketOriginal;
}

void comprobarErrores(int respuestaFuncion, char* llamada, bool cond) {
	if(respuestaFuncion == -1) {
		perror(llamada);
		if(cond) exit(1);
	}
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = tamanio;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, valor, paquete->buffer->size);
}

t_paquete* crear_paquete(int op)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = op;
	crear_buffer(paquete);
	return paquete;
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}

void agregar_a_serializacion(void* magic, int* desplazamiento, void* data, int tamanio){
	memcpy(magic + (*desplazamiento), data, tamanio);
	(*desplazamiento) += tamanio;
}

void* deserializar(int socket, int tamanio, char* info){
	void* data = malloc(tamanio);
	int resul  = recv(socket, data, tamanio, MSG_WAITALL);
	char* msj  = mensajeCompleto("Deserializar ",info);
	comprobarErrores(resul, msj, true);
	free(msj);
	return data;
}

char* mensajeCompleto(char* quienHabla,char* mensaje){
	char* aux = malloc(strlen(quienHabla)+ strlen(mensaje) + 1);
	strcpy(aux,quienHabla);
	strcat(aux,mensaje);
	return aux;
}

