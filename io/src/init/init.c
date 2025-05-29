#include "init.h"

char* IP_KERNEL;
char* PUERTO_KERNEL;
char* LOG_LEVEL;

void inicializar_io(){
  logger = iniciar_logger("io.log", "Kernel", LOG_LEVEL_INFO);
  log_debug(logger, "Log de IO iniciado");
  config = iniciar_config("io.config");
  extraer_datos_config();
};

void extraer_datos_config(){
  IP_KERNEL = config_get_string_value(config, "IP_KERNEL");
  PUERTO_KERNEL = config_get_string_value(config, "PUERTO_KERNEL");
  //LOG_LEVEL = config_get_string_value(config, "LOG_LEVEL");
  log_debug(logger, "Datos extraídos del archivo de configuración");
};