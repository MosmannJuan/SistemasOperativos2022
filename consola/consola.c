#include "consola.h"
int main(int argc, char **argv) {

	ip = strdup(config_get_string_value(consola_config,"IP"));
	puertoKernel = strdup(config_get_string_value(consola_config,"PUERTO"));
}


