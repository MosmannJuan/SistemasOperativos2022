#ifndef CPU_H_
#define CPU_H_


//#include <sharedUtils.c>
#include <commons/log.h>
#include <stdint.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int	entradasTlb ;
char*	reemplazoTlb;
int	retardoNoop;
char*	ipMemoria;
char*	puertoMemoria;
char* 	ipKernel;
char*	puertoEscuchaDispatch;
char*	puertoEscuchaInterrupt;

int conexionMemoria;
int conexionDispatch;
int conexionInterrupt;

t_config* cpu_config;
t_log * cpuLogger;

typedef enum {
	BLOQUEADO,
	INTERRUPCION,
	FINALIZADO
}EstadoPcb;

typedef struct {
	unsigned int id;
    unsigned int tam_proceso;
	EstadoPcb estado;
	t_list * instrucciones;
	unsigned int pc;
	/* TABLA PAGINAS ??? */
	double rafaga;
}Pcb;


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

DireccionLogica* fetch_operands(unsigned int* operandos);
void* decode_execute (Pcb * pcb_decode, Instruccion * instruccion_decode);
void* fetch(Pcb * pcb_fetch);
void abrirArchivoConfiguracion();

#endif /* CPU_H_ */
