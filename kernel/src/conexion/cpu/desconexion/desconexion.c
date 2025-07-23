#include "desconexion.h"

void manejar_desconexion_cpu(int socket, tipo_conexion_cpu tipo){
  pthread_mutex_lock(&mutex_cpus);
  for(int i = 0; i < list_size(lista_cpus); i++){
    t_cpu* cpu = list_get(lista_cpus, i);
    if(tipo == CPU_DISPATCH && cpu->socket_dispatch == socket){
      log_warning(logger, "CPU %d Dispatch desconectada", cpu->id_cpu);
      cpu->socket_dispatch = -1;
    };
    if(tipo == CPU_INTERRUPT && cpu->socket_interrupt == socket){
      log_warning(logger, "CPU %d Interrupt desconectada", cpu->id_cpu);
      cpu->socket_interrupt = -1;
    };
  };
  pthread_mutex_unlock(&mutex_cpus);
};