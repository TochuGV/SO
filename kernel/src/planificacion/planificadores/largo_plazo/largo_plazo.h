#ifndef LARGO_PLAZO_H_
#define LARGO_PLAZO_H_

#include "pcb/pcb.h"
#include "logger/logger.h"
#include "common/common.h"
#include "utils/utils.h" // Revisar después
#include <commons/collections/queue.h>
#include <semaphore.h>

extern t_queue* cola_new;
extern pthread_mutex_t mutex_new;  
extern sem_t hay_procesos_en_new;

void iniciar_planificacion_largo_plazo();
void inicializar_proceso(t_pcb*);
void mover_proceso_a_ready(char*, int32_t);

#endif