#ifndef PCB_H
#define PCB_H

#include <stdint.h>

typedef enum {
  ESTADO_NEW,
  ESTADO_READY,
  ESTADO_EXEC,
  ESTADO_BLOCKED,
  ESTADO_SUSP_BLOCKED,
  ESTADO_SUSP_READY,
  ESTADO_EXIT,
  CANTIDAD_ESTADOS
} t_estado;

typedef struct {
	uint32_t pid; //identificador del proceso
	uint32_t pc; //contador de programa
	uint32_t me[CANTIDAD_ESTADOS]; //cantidad de veces en un estado
	uint32_t mt[CANTIDAD_ESTADOS]; //tiempo que permanecio en ese estado en milisegundos
} t_pcb;

uint32_t generar_pid();
t_pcb* crear_pcb();
void destruir_pcb(t_pcb*);
void* serializar_pcb(t_pcb* pcb, int bytes);

#endif