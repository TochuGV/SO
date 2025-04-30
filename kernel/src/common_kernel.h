#ifndef COMMON_KERNEL_H_
#define COMMON_KERNEL_H_

#include "./utils/utils.h"
#include <stdint.h>

//typedef uint32_t t_pid;
//typedef uint32_t t_pc;

extern const uint32_t CANTIDAD_ESTADOS;

typedef enum {
  NEW,
  READY,
  EXEC,
  BLOCKED,
  SUSP_BLOCKED,
  SUSP_READY,
  FINALIZADO
} t_estado;

typedef struct {
	uint32_t pid; //identificador del proceso
	uint32_t pc; //contador de programa
	uint32_t* me; //cantidad de veces en un estado
	uint32_t* mt; //tiempo que permanecio en ese estado en milisegundos
} t_pcb;


t_pcb* crear_pcb();
void destruir_pcb(t_pcb* pcb);
void* serializar_pcb(t_pcb* pcb, int bytes);
void* enviar_proceso_a_memoria(char* path, uint32_t tamanio_proceso, int socket_cliente);

#endif /* COMMON_KERNEL_H_ */