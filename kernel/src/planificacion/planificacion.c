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
  pthread_create(&hilo_suspended_blocked, NULL, planificador_suspended_blocked, NULL);
  pthread_create(&hilo_suspended_ready, NULL, planificador_suspended_ready, NULL);
  pthread_join(hilo_suspended_blocked, NULL);
  pthread_join(hilo_suspended_ready, NULL);
  return NULL;
};


void* planificador_suspended_blocked(void* arg){
  while(1){
    sem_wait(&semaforo_revisar_bloqueados);
    revisar_bloqueados();
  };
};

void* planificador_suspended_ready(void* arg){
  while(1){
    sem_wait(&semaforo_revisar_susp_ready);
    desuspender_proceso();
  };
};