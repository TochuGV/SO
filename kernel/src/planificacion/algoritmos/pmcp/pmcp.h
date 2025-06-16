#ifndef PMCP_H
#define PMCP_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <commons/collections/queue.h>
#include "planificacion/planificadores/largo_plazo/largo_plazo.h"

t_pcb* elegir_proceso_mas_chico(t_queue*, t_list*);

#endif /* PMCP_H */