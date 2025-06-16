#ifndef LARGO_PLAZO_H_
#define LARGO_PLAZO_H_

#include "pcb/pcb.h"
#include "logger/logger.h"
#include "common/common.h"
#include "utils/utils.h" // Revisar después
#include "./planificacion/algoritmos/pmcp/pmcp.h"
#include <commons/collections/queue.h>
#include <semaphore.h>

typedef struct {
  uint32_t pid;
  uint32_t tamanio;
} t_informacion_largo_plazo;

extern t_queue* cola_new;
extern t_list* lista_tamanios;
extern pthread_mutex_t mutex_new;  
extern sem_t hay_procesos_en_new;

void iniciar_planificacion_largo_plazo();
void inicializar_proceso(t_pcb*, uint32_t);
void mover_proceso_a_ready(char*, int32_t);
void finalizar_proceso(t_pcb*);

#endif