#include "pcb.h"
#include <stdlib.h>
#include <string.h>

int contadorPid = 0;

t_pcb* crear_pcb(){
  t_pcb* pcb = malloc(sizeof(t_pcb));
  if(!pcb) return NULL;

  pcb->pid = contadorPid++;
  pcb->pc = 0;
  pcb->me = calloc(CANTIDAD_ESTADOS, sizeof(uint32_t));
  pcb->mt = calloc(CANTIDAD_ESTADOS, sizeof(uint32_t));

  return pcb;
};

void destruir_pcb(t_pcb* pcb){
  if(!pcb) return;

  free(pcb->me);
  free(pcb->mt);
  free(pcb);
}

void* serializar_pcb(t_pcb* pcb, int bytes){
  /*
  int tamanio_me = sizeof(uint32_t) * 7;
  int tamanio_mt = sizeof(uint32_t) * 7;
  int tamanio = sizeof(uint32_t) * 2 + tamanio_me + tamanio_mt;

  Se pueden calcular los bytes antes de llamar a la función, o calcularlos dentro de la función o crear otra.
  Si se calculan antes, el parámetro es 'int bytes', sino tendría que ser 'int* bytes'.
  */

  void* magic = malloc(bytes);
  int desplazamiento = 0;

  memcpy(magic + desplazamiento, &(pcb->pid), sizeof(uint32_t));
  desplazamiento += sizeof(uint32_t);
  memcpy(magic + desplazamiento, &(pcb->pc), sizeof(uint32_t));
  desplazamiento += sizeof(uint32_t);
  memcpy(magic + desplazamiento, &(pcb->me), sizeof(uint32_t) * 7);
  desplazamiento += sizeof(uint32_t) * 7;
  memcpy(magic + desplazamiento, &(pcb->mt), sizeof(uint32_t) * 7);
  desplazamiento += sizeof(uint32_t) * 7;

  return magic;
}
/*
void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

void* serializar_mensaje(char* mensaje, int bytes)
{
  void * magic = malloc(bytes);
  int desplazamiento = 0;
  
  memcpy(magic + desplazamiento, &(mensaje), sizeof(int));
  desplazamiento+= sizeof(int);
  return magic;
}
*/
