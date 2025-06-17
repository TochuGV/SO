#include "planificacion.h"

void* planificador_ciclo_general(void* arg){
  while(1){
    mover_proceso_a_ready();
    sem_wait(&semaforo_ready);
    sem_wait(&semaforo_cpu_libre);
    mover_proceso_a_exec();
    sleep(1);
  };
};