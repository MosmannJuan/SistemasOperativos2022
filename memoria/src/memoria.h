#ifndef MEMORIA_H_
#define MEMORIA_H_


#include <stdio.h>
#include <stdlib.h>
#include <sharedUtils.h>
#include <commons/log.h>
#include <stdint.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <unistd.h>

t_config* memoria_config;
#endif /* MEMORIA_H_ */

/////FIJARSE SI CAMBIAR A uint32_t
char* ipMemoria;
char* puertoEscucha;
int tamMemoria;
int tamPagina;
int tamTabla;
int paginasPorTabla;
int retardoMemoria;
int marcosPorProceso;
int retardoSwap;
void* baseMemoria;
char* pathSwap;
t_list* memoriaPrimerNivelList;
pthread_mutex_t mutexMemoria;
t_log *loggerMemoria;
char* algoritmoReemplazo;
char* pathSwap;

int conexion;


typedef struct memoriaSegundoNivel
{
	int id;
	uint32_t marco;
	char* presencia;
	char* uso;
	char* modificado;
} memoriaSegundoNivel;

typedef struct memoriaPrimerNivel
{
	int id;
	void* nroProceso;
	t_list* memoriaSegundoNivelList;
} memoriaPrimerNivel;

typedef struct PaginaEspesifica
{
	int idTabla;
	int idMarco;
} PaginaEspesifica;


void abrirArchivoConfifuracion();
void configurarMemoria();
void atenderMensajes();
void borrar(int pagina, int marco);
bool obtenerPaginaVacia(memoriaSegundoNivel *pagina);
bool obtenerTablaVacia(memoriaPrimerNivel *tabla);
void crearProceso(int procesId);
PaginaEspesifica* obtenerPaginaASwapear();
PaginaEspesifica* swappear(int dato, int pid,PaginaEspesifica* paginaASwappear);
PaginaEspesifica* escribir(int dato, int pid);
PaginaEspesifica* escirbirSinSwap(int dato, int pid);

