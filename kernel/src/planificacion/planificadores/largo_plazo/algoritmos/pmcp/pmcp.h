#ifndef PMCP_H_
#define PMCP_H_

#include "planificacion/planificadores/largo_plazo/largo_plazo.h"
#include "pcb/pcb.h"
#include "common/common.h"
#include "planificacion/estados/estados.h"
#include <commons/collections/queue.h>
#include <semaphore.h>

void mover_proceso_a_ready_pmcp(void);

#endif /* PMCP_H */