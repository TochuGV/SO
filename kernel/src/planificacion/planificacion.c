#include "planificacion.h"

void* planificador_ciclo_general(void* arg){
  while(1){
    mover_proceso_a_ready(NULL, 0);
    mover_proceso_a_exec();
    sleep(1);
  };
};