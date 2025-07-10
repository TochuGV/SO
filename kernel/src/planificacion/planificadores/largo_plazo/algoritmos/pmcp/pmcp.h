#ifndef PMCP_H_
#define PMCP_H_

#include "planificacion/planificadores/largo_plazo/largo_plazo.h"
#include "planificacion/planificadores/largo_plazo/algoritmos/algoritmos.h"
#include "pcb/pcb.h"
#include <commons/collections/queue.h>
#include <semaphore.h>

void mover_proceso_a_ready_pmcp(void);

#endif /* PMCP_H */