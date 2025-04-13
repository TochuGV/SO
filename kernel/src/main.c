#include "main.h"

int main(int argc, char* argv[]) 
{
  
  logger = iniciar_logger("kernel.log", "Kernel", LOG_LEVEL_DEBUG);
  log_info(logger, "Log de Kernel iniciado");

  config = iniciar_config("kernel.config");
	
  t_list* lista_puertos = list_create();
  agregar_puertos_a_lista(KERNEL, config, lista_puertos);

  conectar_modulos(lista_puertos);

  //terminar_programa(conexion, logger, config);

	return EXIT_SUCCESS;
  
}

