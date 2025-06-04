#include "main.h"

int main(int argc, char* argv[]){
  inicializar_memoria();
  
  while (1) {
    int socket_cliente = accept(servidor_memoria, NULL, NULL);
    if (socket_cliente == -1) {
      log_error(logger, "Error al aceptar conexión");
      continue;
    }
    int* socket_ptr = malloc(sizeof(int));
    *socket_ptr = socket_cliente;
    pthread_t hilo_atender;
    pthread_create(&hilo_atender, NULL, atender_cliente, socket_ptr);
    pthread_detach(hilo_atender); // si no te interesa hacer join
  }
  terminar_memoria();
  return EXIT_SUCCESS;
};

