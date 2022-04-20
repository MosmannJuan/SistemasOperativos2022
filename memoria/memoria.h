#ifndef MEMORIA_H_
#define MEMORIA_H_


#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <stdint.h>
#include<commons/collections/list.h>
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
char* algoritmoReemplazo;
char* pathSwap;

int conexion;


void abrirArchivoConfifuracion();
void configurarMemoria();
void atenderMensajes();


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
	void* numeroPagina;
	t_list* memoriaSegundoNivelList;
} memoriaPrimerNivel;

t_list* memoriaPrimerNivelList;

