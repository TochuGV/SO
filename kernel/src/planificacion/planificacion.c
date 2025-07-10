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
    //log_warning(logger, "SEM READY");
    sem_wait(&semaforo_cpu_libre);
    //log_warning(logger, "SEM CPU LIBRE");
    mover_proceso_a_exec();
  };
};

void* planificador_mediano_plazo(void* arg){
  while(1){
    sleep(1);
  };
};