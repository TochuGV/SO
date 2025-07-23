#include "init.h"

char* IP_KERNEL;
char* PUERTO_KERNEL;
char* LOG_LEVEL;

void inicializar_io(){
  config = iniciar_config("io.config");
  extraer_datos_config();
  logger = iniciar_logger("io.log", "Kernel", parse_log_level(LOG_LEVEL));
  log_debug(logger, "Log de IO iniciado");
};

void extraer_datos_config(){
  IP_KERNEL = config_get_string_value(config, "IP_KERNEL");
  PUERTO_KERNEL = config_get_string_value(config, "PUERTO_KERNEL");
  LOG_LEVEL = config_get_string_value(config, "LOG_LEVEL");
};