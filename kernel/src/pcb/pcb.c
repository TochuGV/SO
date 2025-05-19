#include "pcb.h"
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

static uint32_t ultimo_pid = 0;
static pthread_mutex_t mutex_pid = PTHREAD_MUTEX_INITIALIZER;

uint32_t generar_pid(){
  pthread_mutex_lock(&mutex_pid);
  uint32_t nuevo_pid = ultimo_pid++;
  pthread_mutex_unlock(&mutex_pid);
  return nuevo_pid;
};

t_pcb* crear_pcb(){
  t_pcb* nuevo_pcb = malloc(sizeof(t_pcb));
  if(!nuevo_pcb) return NULL;

  nuevo_pcb->pid = generar_pid();
  nuevo_pcb->pc = 0;

  for(int i = 0; i < CANTIDAD_ESTADOS; i++){
    nuevo_pcb->me[i] = 0;
    nuevo_pcb->mt[i] = 0;
  };

  return nuevo_pcb;
};

void destruir_pcb(t_pcb* pcb){
  if(!pcb) return; //Revisar validación acá
  free(pcb);
};

void* serializar_pcb(t_pcb* pcb, int bytes){
  /*
  int tamanio_me = sizeof(uint32_t) * 7;
  int tamanio_mt = sizeof(uint32_t) * 7;
  int tamanio = sizeof(uint32_t) * 2 + tamanio_me + tamanio_mt;

  Se pueden calcular los bytes antes de llamar a la función, o calcularlos dentro de la función o crear otra.
  Si se calculan antes, el parámetro es 'int bytes', sino tendría que ser 'int* bytes'.

  Ejemplo de uso:
    int bytes = sizeof(uint32_t) * (2 + CANTIDAD_ESTADOS * 2);
    void* buffer = serializar_pcb(pcb, bytes);
  */

  void* magic = malloc(bytes);
  int desplazamiento = 0;

  memcpy(magic + desplazamiento, &(pcb->pid), sizeof(uint32_t));
  desplazamiento += sizeof(uint32_t);
  
  memcpy(magic + desplazamiento, &(pcb->pc), sizeof(uint32_t));
  desplazamiento += sizeof(uint32_t);
  
  memcpy(magic + desplazamiento, pcb->me, sizeof(uint32_t) * CANTIDAD_ESTADOS);
  desplazamiento += sizeof(uint32_t) * CANTIDAD_ESTADOS;
  
  memcpy(magic + desplazamiento, pcb->mt, sizeof(uint32_t) * CANTIDAD_ESTADOS);
  desplazamiento += sizeof(uint32_t) * CANTIDAD_ESTADOS;

  return magic;
};

char* obtener_nombre_estado(t_estado estado){
  switch(estado){
    case ESTADO_NEW: return "NEW";
    case ESTADO_READY: return "READY";
    case ESTADO_EXEC: return "EXEC";
    case ESTADO_BLOCKED: return "BLOCKED";
    case ESTADO_SUSP_BLOCKED: return "SUSPENDED_BLOCKED";
    case ESTADO_SUSP_READY: return "SUSPENDED_READY";
    case ESTADO_EXIT: return "EXIT";
    default: return "UNKNOWN";
  };
};