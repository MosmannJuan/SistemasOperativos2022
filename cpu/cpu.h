#ifndef CPU_H_
#define CPU_H_
#include <stdio.h>
#include <stdlib.h>
#include <sharedUtils.c>
#include <commons/log.h>
#include <stdint.h>
#include <commons/collections/list.h>
#include <commons/config.h>

int	entradasTlb ;
char*	reemplazoTlb;
int	retardoNoop;
char*	ipMemoria;
int	puertoMemoria;
int	puertoEscuchaDispatch;
int	puertoEscuhcaInterrupt;

int conexionDispatch;
int conexionInterrupt;

t_config* cpuConfig;

typedef struct {
	unsigned int id;
    unsigned int tam_proceso;
	EstadoPcb estado;
	t_list * instrucciones;
	unsigned int pc;
	/* TABLA PAGINAS ??? */
	double rafaga;
}Pcb;

void* fetch( Pcb pcb);

typedef enum {
	BLOQUEADO,
	INTERRUPCION,
	FINALIZADO
}EstadoPcb;

typedef enum {
	NO_OP,
	I_O,
	READ,
	WRITE,
	COPY,
	EXIT
}TipoInstruccion;

typedef struct Instruccion {
	TipoInstruccion tipo;
	unsigned int params[2];
}Instruccion;

typedef struct DireccionLogica{
	int primerNivel;
	int segundoNivel;
	int desplazamiento;
}DireccionLogica;

DireccionLogica	fetchOperands(int[]);
void* decodeExecute(Instruccion);

#endif /* CPU_H_ */
