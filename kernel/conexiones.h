/*

#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include <string.h>
#include <readline/readline.h>
#include <pthread.h>
#include <sys/types.h>


typedef enum
{
	MENSAJE,
	SEM_INIT,
	SEM_WAIT,
	SEM_POST,
	SEM_DESTROY,
	IO,
	MENSAJE_MEMORIA,
	MEM_ALLOC,
	MEM_FREE,
	MEM_READ,
	MEM_WRITE,
	GUARDAR,
	BUSCAR,
	ELIMINAR,
	COSITA,
	PAQUETE,
	END
}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

///////////////////////
//FUNCIONES GENERALES//
///////////////////////

struct addrinfo* setDireccion(struct addrinfo* hints, char* ip, char* puerto, struct addrinfo* server_info);
int crearSocket(struct addrinfo *servinfo);
void comprobarErrores(int respuestaFuncion, char* llamada, bool cond);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
t_paquete* crear_paquete(int op);
void eliminar_paquete(t_paquete* paquete);
void* serializar_paquete(t_paquete* paquete, int bytes);
void liberar_conexion(int socket_cliente);
void agregar_a_serializacion(void* magic, int* desplazamiento, void* data, int tamanio);
void* deserializar(int socket, int tamanio, char* info);
char* mensajeCompleto(char* quienHabla,char* mensaje);

#endif /* CONEXIONES_H_ */
