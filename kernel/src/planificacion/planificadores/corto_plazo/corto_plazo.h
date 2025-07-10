#ifndef CORTO_PLAZO_H_
#define CORTO_PLAZO_H_

#include "pcb/pcb.h"
#include "logger/logger.h"
#include "common/common.h"
#include "utils/utils.h" // Revisar después
#include <commons/collections/queue.h>
#include <semaphore.h>
#include "cpu/cpu.h"

#include "planificacion/planificadores/corto_plazo/algoritmos/fifo/fifo.h"
#include "planificacion/planificadores/corto_plazo/algoritmos/sjf/sjf.h"
#include "planificacion/planificadores/corto_plazo/algoritmos/srt/srt.h"

extern t_queue* cola_ready;
extern sem_t semaforo_ready;
extern pthread_mutex_t mutex_ready;   

void iniciar_planificacion_corto_plazo();
void mover_proceso_a_exec();
void enviar_a_cpu(t_pcb*, int);

#endif /* CORTO_PLAZO_H_ */