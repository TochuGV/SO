#include "planificacion.h"

void* planificador_ciclo_general(void* arg){
  while(1){
    /*
    // Seguir intentando mover procesos de NEW a READY mientras se pueda
    while(true) {
      int cantidad_antes, cantidad_despues;

      pthread_mutex_lock(&mutex_new);
      cantidad_antes = queue_size(cola_new);
      pthread_mutex_unlock(&mutex_new);

      mover_proceso_a_ready(NULL, 0);  // Intenta mover uno

      pthread_mutex_lock(&mutex_new);
      cantidad_despues = queue_size(cola_new);
      pthread_mutex_unlock(&mutex_new);

      // Si no se movió nadie, cortar
      if (cantidad_despues == cantidad_antes)
        break;
    }
    */
    mover_proceso_a_ready(NULL, 0);
    sem_wait(&semaforo_ready);
    sem_wait(&semaforo_cpu_libre);
    mover_proceso_a_exec();
    sleep(1);
  };
};