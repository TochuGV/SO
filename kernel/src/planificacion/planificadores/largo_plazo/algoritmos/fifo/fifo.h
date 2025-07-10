#ifndef LP_FIFO_H_
#define LP_FIFO_H_

#include "planificacion/planificadores/largo_plazo/largo_plazo.h"
#include "planificacion/planificadores/largo_plazo/algoritmos/algoritmos.h"
#include "pcb/pcb.h"
#include <commons/collections/queue.h>
#include <semaphore.h>

void mover_proceso_a_ready_fifo(void);

#endif /* FIFO_H */