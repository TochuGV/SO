#include "main.h"

int main(int argc, char* argv[]) 
{
  inicializar_memoria();
  
  while (1) {
    int* cliente_memoria = malloc(sizeof(int));
    log_debug(logger, "Esperando conexiones en Memoria...");
    *cliente_memoria = accept(servidor_memoria, NULL, NULL);
    pthread_t hilo_atender;
    pthread_create(&hilo_atender, NULL, atender_cliente, cliente_memoria);
  }

  terminar_memoria();

  return EXIT_SUCCESS;

}

