#include "consola.h"

void leerArchivoDeInstrucciones(char * pathArchivoInstrucciones) {
  char palabraLeida[10] = "\0";
  char c;
  char control = '0';
  Instruccion instruccionAux;
  instruccionAux.params[0] = 0;
  instruccionAux.params[1] = 0;

  //Abrir archivo
  FILE * file = fopen(pathArchivoInstrucciones, "r+");

  if (!file) {
    //Debería usar el logger para los errores
    log_info(logger, "No se pudo abrir el archivo de instrucciones solicitado");
  }
  //Cicla el archivo leyendo las instrucciones hasta llegar al EOF
  while (control != EOF) {
    c = fgetc(file);
    int contadorCaracter = 0;
    //Lee el identificador de la instrucción
    while (c != ' ') {
      palabraLeida[contadorCaracter] = c;
      c = fgetc(file);
      contadorCaracter++;
      if (strcmp(palabraLeida, "EXIT") == 0) break;
    }

    //Arma la instrucción en función del identificador leído

    if (strcmp(palabraLeida, "NO_OP") == 0) {
      instruccionAux.tipo = NO_OP;
    }

    if (strcmp(palabraLeida, "I/O") == 0) {
      instruccionAux.tipo = I_O;
    }

    if (strcmp(palabraLeida, "READ") == 0) {
      instruccionAux.tipo = READ;
    }

    if (strcmp(palabraLeida, "WRITE") == 0) {
      instruccionAux.tipo = WRITE;
    }

    if (strcmp(palabraLeida, "COPY") == 0) {
      instruccionAux.tipo = COPY;
    }

    if (strcmp(palabraLeida, "EXIT") == 0) {
      instruccionAux.tipo = EXIT;
    }

    lectura_y_asignacion_parametros( & instruccionAux, file);

    memset(palabraLeida, '\0', 10);

    enviar_instruccion(instruccionAux);

    //Consume el EOF o \n
    control = fgetc(file);

  }

}

void lectura_y_asignacion_parametros(Instruccion * instruccionAux, FILE * file) {
  int i = 0;
  switch (instruccionAux -> tipo) {
  case I_O:
  case NO_OP:
  case READ:
    lectura_y_asignacion_un_parametro(instruccionAux, file, i);
    break;
  case WRITE:
  case COPY:
    lectura_y_asignacion_dos_parametro(instruccionAux, file, i);
    break;
  case EXIT:
  	  break;
  }
}

void lectura_y_asignacion_un_parametro(Instruccion * instruccionAux, FILE * file, int i) {
  fscanf(file, "%d", & i);
  instruccionAux -> params[0] = i;

}

void lectura_y_asignacion_dos_parametro(Instruccion * instruccionAux, FILE * file, int i) {
  lectura_y_asignacion_un_parametro(instruccionAux, file, i);
  fgetc(file);
  fscanf(file, "%d", & i);
  instruccionAux -> params[1] = i;
}

void enviar_instruccion(Instruccion instruccion) {
  send(conexion, & instruccion.tipo, sizeof(int), 0);
  switch (instruccion.tipo) {
  case I_O:
  case NO_OP:
  case READ:
    send(conexion, & instruccion.params[0], sizeof(unsigned int), 0);
    break;
  case WRITE:
  case COPY:
    send(conexion, & instruccion.params[0], sizeof(unsigned int), 0);
    send(conexion, & instruccion.params[1], sizeof(unsigned int), 0);
    break;
  case EXIT:
	  break;
  }

}

void terminar_programa(int conexion, t_log * logger, t_config * config) {
  log_destroy(logger);
  config_destroy(config);
  close(conexion);
}

int main(int argc, char ** argv) {

  //Crear logger.
  logger = log_create("errors.log", "error_logger", 1, LOG_LEVEL_ERROR);

  //Levantar config, traer ip y puerto.
  consola_config = config_create("consola.config");
  ip = strdup(config_get_string_value(consola_config, "IP_KERNEL"));
  puertoKernel = strdup(config_get_string_value(consola_config, "PUERTO_KERNEL"));

  //inicializar conexion con kernel.
  conexion = conexion_a_kernel(ip, puertoKernel);

  //Parser de instrucciones
  pathArchivoInstrucciones = argv[1];

  leerArchivoDeInstrucciones("InstruccionTest.txt");

  //Finalizar conexion socket, logger y config.
  terminar_programa(conexion, logger, consola_config);

}
