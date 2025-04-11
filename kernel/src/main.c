#include "main.h"

int main(int argc, char* argv[]) {
  /*
  //int conexion;
  //char* ip;
  //char* puerto;
  //char* valor;
  
  t_log* logger;
  t_config* config;
  
  logger = iniciar_logger();
  log_info(logger, "Log iniciado");

  config = iniciar_config("kernel.config");
  */
  logger = log_create("kernel.log", "Kernel", 1, LOG_LEVEL_DEBUG);

  int server_fd = iniciar_servidor();
	log_info(logger, "Kernel listo para recibir al cliente");
	int cliente_fd = esperar_cliente(server_fd);
  
  t_list* lista;
	while (1) {
		int cod_op = recibir_operacion(cliente_fd);
		switch (cod_op) {
    case MENSAJE:
			recibir_mensaje(cliente_fd);
			break;
		case PAQUETE:
			lista = recibir_paquete(cliente_fd);
			log_info(logger, "Me llegaron los siguientes valores:\n");
			list_iterate(lista, (void*) iterator);
			break;
		case -1:
			log_error(logger, "El cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(logger,"Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
	return EXIT_SUCCESS;
}

void iterator(char* value) {
	log_info(logger, "%s", value);
}