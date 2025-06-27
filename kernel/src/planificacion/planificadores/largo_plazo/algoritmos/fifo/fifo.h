#ifndef FIFO_H_
#define FIFO_H_

#include "planificacion/planificadores/largo_plazo/largo_plazo.h"
#include "pcb/pcb.h"
#include "common/common.h"
#include "planificacion/estados/estados.h"
#include <commons/collections/queue.h>
#include <semaphore.h>

void mover_proceso_a_ready_fifo(void);

#endif /* FIFO_H */