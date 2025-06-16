#ifndef CORTO_PLAZO_H_
#define CORTO_PLAZO_H_

#include "pcb/pcb.h"
#include "logger/logger.h"
#include "common/common.h"
#include "utils/utils.h" // Revisar después
#include <commons/collections/queue.h>
#include <semaphore.h>

extern t_queue* cola_ready;
extern sem_t semaforo_ready;
extern pthread_mutex_t mutex_ready;   
extern pthread_mutex_t mutex_cpus;

void iniciar_planificacion_corto_plazo();
void mover_proceso_a_exec();
void enviar_a_cpu(t_pcb*, int);
void liberar_cpu(uint32_t);
void liberar_cpu_por_pid(uint32_t);

#endif