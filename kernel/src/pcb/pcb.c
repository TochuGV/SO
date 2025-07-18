#include "pcb.h"
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

uint32_t ultimo_pid = 0;
pthread_mutex_t mutex_pid = PTHREAD_MUTEX_INITIALIZER;

uint32_t generar_pid(){
  pthread_mutex_lock(&mutex_pid);
  uint32_t nuevo_pid = ultimo_pid++;
  pthread_mutex_unlock(&mutex_pid);
  return nuevo_pid;
};

t_pcb* crear_pcb(){
  t_pcb* pcb = malloc(sizeof(t_pcb));
  if(!pcb) return NULL;
  pcb->pid = generar_pid();
  pcb->pc = 0;
  for(int i = 0; i < CANTIDAD_ESTADOS - 1; i++){
    pcb->me[i] = 0;
    pcb->mt[i] = 0;
  };
  return pcb;
};

void destruir_pcb(t_pcb* pcb){
  if(!pcb) return;
  free(pcb);
};

void* serializar_pcb(t_pcb* pcb, int* bytes){

  /*
  Se pueden calcular los bytes antes de llamar a la función, o calcularlos dentro de la función o crear otra.
  Si se calculan antes, el parámetro es 'int bytes', sino tendría que ser 'int* bytes'.

  Ejemplo de uso:
    int bytes = sizeof(uint32_t) * 2;
    void* buffer = serializar_pcb(pcb, bytes);
  */

  *bytes = sizeof(uint32_t) * 2;
  void* magic = malloc(*bytes);
  int desplazamiento = 0;
  memcpy(magic + desplazamiento, &(pcb->pid), sizeof(uint32_t));
  desplazamiento += sizeof(uint32_t);
  memcpy(magic + desplazamiento, &(pcb->pc), sizeof(uint32_t));
  desplazamiento += sizeof(uint32_t);
  return magic;
};