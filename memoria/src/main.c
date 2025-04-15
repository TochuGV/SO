#include "main.h"

int main(int argc, char* argv[]) 
{
  int conexion = 0;

  logger = iniciar_logger("memoria.log", "Memoria", LOG_LEVEL_DEBUG);
  log_info(logger, "Log de Memoria iniciado");

  config = iniciar_config("memoria.config");
	
  t_list* lista_puertos = list_create();
  agregar_puertos_a_lista(MEMORIA, config, lista_puertos);

  conectar_puertos_a_servidor(lista_puertos);

  terminar_programa(conexion, logger, config);

  return EXIT_SUCCESS;

}
