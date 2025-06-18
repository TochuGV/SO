#include "planificacion.h"

void* planificador_largo_plazo(void* arg){
  while(1){
    if (ingreso_a_memoria)
      mover_proceso_a_ready();
    sleep(1);
  };
};

void* planificador_corto_plazo(void* arg){
  while(1){
    sem_wait(&semaforo_ready);
    sem_wait(&semaforo_cpu_libre);
    mover_proceso_a_exec();
  };
};