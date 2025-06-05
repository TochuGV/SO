#include "main.h"

int main(int argc, char* argv[]){
  inicializar_memoria();
  
  while (1) {
    int* cliente_memoria = malloc(sizeof(int));
    log_debug(logger, "Esperando conexiones...");
    *cliente_memoria = accept(servidor_memoria, NULL, NULL);
    pthread_t hilo_atender;
    pthread_create(&hilo_atender, NULL, atender_cliente, socket_ptr);
  };
  terminar_memoria();
  return EXIT_SUCCESS;
};