#ifndef PCB_H_
#define PCB_H_

#include <stdint.h>

//typedef uint32_t t_pid;
//typedef uint32_t t_pc;

const uint32_t CANTIDAD_ESTADOS = 7;

typedef enum {
  NEW,
  READY,
  EXEC,
  BLOCKED,
  SUSP_BLOCKED,
  SUSP_READY,
  EXIT
} t_estado;

typedef struct {
	uint32_t pid; //identificador del proceso
	uint32_t pc; //contador de programa
	uint32_t* me; //cantidad de veces en un estado
	uint32_t* mt; //tiempo que permanecio en ese estado en milisegundos
} t_pcb;

#endif /* PCB_H_ */