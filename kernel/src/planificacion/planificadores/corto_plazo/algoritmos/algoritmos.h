#ifndef CP_ALGORITMOS_H
#define CP_ALGORITMOS_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include "utils/utils.h"
#include "init/init.h"

t_pcb* obtener_proximo_proceso_ready(t_queue* cola_ready);
void asignar_y_enviar_a_cpu(t_pcb* pcb, t_cpu* cpu);

#endif /* CP_ALGORITMOS_H */