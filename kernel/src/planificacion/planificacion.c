#include "planificacion.h"

void* planificador_largo_plazo(void* arg){
  while(1){
    sem_wait(&semaforo_revisar_largo_plazo);
    mover_proceso_a_ready();
  };
};

void* planificador_corto_plazo(void* arg){
  while(1){
    sem_wait(&semaforo_ready);
    sem_wait(&semaforo_cpu_libre);
    mover_proceso_a_exec();
  };
};

void* planificador_mediano_plazo(void* arg){
  while(1){
    // semaforo para acceder a suspender
    sleep(1);
  };
};