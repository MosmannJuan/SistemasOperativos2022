#include "consola.h"



int main(int argc, char ** argv) {

  //Crear logger.
  logger = log_create("errors.log", "error_logger", 1, LOG_LEVEL_ERROR);

  consola_config = config_create("consola.config");
  ip = strdup(config_get_string_value(consola_config, "IP_KERNEL"));
  puerto_kernel = strdup(config_get_string_value(consola_config, "PUERTO_KERNEL"));

  //inicializar conexion con kernel.
  int conexion = conexion_a_kernel(ip, puerto_kernel);

  //Parser de instrucciones
  path_archivo_instrucciones = argv[1];

  leer_y_enviar_archivo_de_instrucciones("InstruccionesTest2.txt", logger, conexion);

  //Finalizar conexion socket, logger y config.
   terminar_programa(conexion, logger, consola_config);

   }
