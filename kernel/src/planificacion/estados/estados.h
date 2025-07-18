#ifndef ESTADOS_H_
#define ESTADOS_H_

#include "pcb/pcb.h"
#include "logger/logger.h"
#include "common/common.h"
#include "utils/utils.h" // Revisar después
#include <commons/collections/queue.h>

void entrar_estado(t_pcb*, t_estado);
void cambiar_estado(t_pcb*, t_estado, t_estado);

#endif