#include "consola.h"

int main(int argc, char ** argv) {

  //Crear logger.
  logger = log_create("errors", "error_logger", 1, LOG_LEVEL_ERROR);

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
  close(conexion);
  log_destroy(logger);
  config_destroy(consola_config);
}

void leerArchivoDeInstrucciones(char * pathArchivoInstrucciones) {
  char * palabraLeida;
  char c;
  char control = '0';
  Instruccion instruccionAux;
  instruccionAux.params[0] = 0;
  instruccionAux.params[1] = 0;
  listaInstrucciones = list_create();

  //Abrir archivo
  FILE * file = fopen(pathArchivoInstrucciones, "r+");

  if (!file) {
    //Debería usar el logger para los errores
    log_info(logger, "Instruction file can't be opened");
  }
  //Cicla el archivo leyendo las instrucciones hasta llegar al EOF
  while (control != EOF) {
    c = fgetc(file);

    //Lee el identificador de la instrucción
    while (c != ' ') {
      strncat(palabraLeida, & c, 1);
    }

    //Arma la instrucción en función del identificador leído
    if (strcmp(palabraLeida, "NO_OP") == 0) {
      instruccionAux.tipo = NO_OP;
      int i = 0;
      fscanf(pathArchivoInstrucciones, "%d", & i);
      instruccionAux.params[0] = i;
    }

    if (strcmp(palabraLeida, "I/O") == 0) {
      instruccionAux.tipo = I_O;
      int i = 0;
      fscanf(pathArchivoInstrucciones, "%d", & i);
      instruccionAux.params[0] = i;
    }

    if (strcmp(palabraLeida, "READ") == 0) {
      instruccionAux.tipo = READ;
      int i = 0;
      fscanf(pathArchivoInstrucciones, "%d", & i);
      instruccionAux.params[0] = i;
    }

    if (strcmp(palabraLeida, "WRITE") == 0) {
      instruccionAux.tipo = WRITE;
      int i = 0;
      fscanf(pathArchivoInstrucciones, "%d", & i);
      instruccionAux.params[0] = i;
      fgetc(pathArchivoInstrucciones);
      fscanf(pathArchivoInstrucciones, "%d", & i);
      instruccionAux.params[1] = i;
    }

    if (strcmp(palabraLeida, "COPY") == 0) {
      instruccionAux.tipo = COPY;
      int i = 0;
      fscanf(pathArchivoInstrucciones, "%d", & i);
      instruccionAux.params[0] = i;
      fgetc(pathArchivoInstrucciones);
      fscanf(pathArchivoInstrucciones, "%d", & i);
      instruccionAux.params[1] = i;
    }

    if (strcmp(palabraLeida, "EXIT") == 0) {
      instruccionAux.tipo = EXIT;
    }

    //Añade la instrucción a la lista
    list_add(listaInstrucciones, & instruccionAux);

    //Consume el EOF o \n
    control = fgetc(file);

  }
}
