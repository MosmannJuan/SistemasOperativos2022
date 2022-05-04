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


void abrirArchivoConfifuracion();
void configurarMemoria();
void atenderMensajes();
void borrar(int pagina, int marco);
paginaEspesifica* obtenerPaginaASwapear();
paginaEspesifica* swappear(int dato, int pid,paginaEspesifica* paginaASwappear);
paginaEspesifica* escribir(int dato, int pid);
paginaEspesifica* escirbirSinSwap();

typedef struct memoriaSegundoNivel
{
	int id;
	uint32_t marco;
	char* presencia;
	char* uso;
	char* modificado;
	int pid;
} memoriaSegundoNivel;

typedef struct memoriaPrimerNivel
{
	int id;
	void* nroProceso;
	t_list* memoriaSegundoNivelList;
} memoriaPrimerNivel;

typedef struct memoriaPrimerNivel
{
	int idTabla;
	int idMarco;
} paginaEspesifica;
