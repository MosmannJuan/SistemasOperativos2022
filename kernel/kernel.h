
#ifndef KERNEL_H_
#define KERNEL_H_

#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

typedef enum {
  NO_OP,
  I_O,
  READ,
  WRITE,
  COPY,
  EXIT
}
TipoInstruccion;

typedef struct Instruccion {
  TipoInstruccion tipo;
  unsigned int params[2];
}
Instruccion;

char * ipMemoria;
char * ipKernel;
int puertoMemoria;
char * ipCpu;
char * algoritmoPlanificacion;
int puertoCpuInterrupt;
int puertoCpuDispatch;
char * puertoEscucha;
int estimacionInicial;
int alfa;
int gradoMultiprogramacion;
int tiempoMaximoBloqueado;
pid_t procesoHijo;

typedef struct {
  int conn_kernel;
  //int conn_memoria;
  //char *ipMemoria;
  //char *puertoMemoria;
}
conexiones;

t_config * kernel_config;
int recibir_int(int socket_cliente);
void atender_instrucciones_cliente(int cliente_fd, t_list * instrucciones);


#endif /* KERNEL_H_ */
