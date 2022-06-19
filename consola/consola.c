#include "consola.h"



int main(int argc, char ** argv) {

  //Crear logger.
  error_logger = log_create("errors.log", "error_logger", 1, LOG_LEVEL_ERROR);
  info_logger = log_create("consola.log", "consola_info", 1, LOG_LEVEL_INFO);

  consola_config = config_create("consola.config");
  ip = strdup(config_get_string_value(consola_config, "IP_KERNEL"));
  puerto_kernel = strdup(config_get_string_value(consola_config, "PUERTO_KERNEL"));

  //inicializar conexion con kernel.
  int conexion = conexion_a_kernel(ip, puerto_kernel);

  //Parser de instrucciones
  path_archivo_instrucciones = argv[1];

  leer_y_enviar_archivo_de_instrucciones("InstruccionTest3.txt", error_logger, conexion);

  bool finalizo_exitosamente;
  recv(conexion, &finalizo_exitosamente, sizeof(bool), 0);

  if(finalizo_exitosamente){
	  log_info(info_logger, "El proceso finalizó exitosamente!");
  }else{
	  log_error(error_logger, "Ha ocurrido un error y el proceso ha finalizado de manera forzada. :'(");
  }

  //Finalizar conexion socket, logger y config.
   terminar_programa(conexion, error_logger, info_logger, consola_config);



   }
