#include "planificacion.h"

void* planificador_largo_plazo(void* arg){
  while(1){
    sem_wait(&semaforo_revisar_largo_plazo);
    mover_proceso_a_ready();
  };
};

void* planificador_corto_plazo(void* arg){
  while(1){
    //log_warning(logger, "Esperando 'semaforo_ready'");
    sem_wait(&semaforo_ready);
    //log_warning(logger, "Esperando 'semaforo_cpu_libre'");

    //int valor_cpu_libre;
    //sem_getvalue(&semaforo_cpu_libre, &valor_cpu_libre);
    //log_debug(logger, "[planificacion.c - Antes de sem_wait] Semáforo CPU libre: %d", valor_cpu_libre);

    sem_wait(&semaforo_cpu_libre);

    //sem_getvalue(&semaforo_cpu_libre, &valor_cpu_libre);
    //log_debug(logger, "[planificacion.c - Después de sem_wait] Semáforo CPU libre: %d", valor_cpu_libre);
    
    //log_warning(logger, "Moviendo el proceso a EXECUTE");
    mover_proceso_a_exec();
  };
};

void* planificador_mediano_plazo(void* arg){
  while(1){
    sem_wait(&semaforo_revisar_susp_ready);
    desuspender_proceso();
  };
};
