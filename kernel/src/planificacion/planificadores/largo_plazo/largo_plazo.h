#ifndef LARGO_PLAZO_H_
#define LARGO_PLAZO_H_

#include "pcb/pcb.h"
#include "logger/logger.h"
#include "common/common.h"
#include "utils/utils.h" // Revisar después

#include "planificacion/planificadores/largo_plazo/algoritmos/fifo/fifo.h"
#include "planificacion/planificadores/largo_plazo/algoritmos/pmcp/pmcp.h"

#include "./planificacion/algoritmos/sjf/sjf.h"
#include <commons/collections/queue.h>
#include <semaphore.h>
#include "./planificacion/algoritmos/srt/srt.h"

typedef struct {
  uint32_t pid;
  char* archivo_pseudocodigo;
  uint32_t tamanio;
} t_informacion_largo_plazo;

extern t_queue* cola_new;
extern t_list* lista_info_procesos;
extern pthread_mutex_t mutex_new;  
//extern sem_t hay_procesos_en_new;
extern sem_t semaforo_revisar_largo_plazo;

void iniciar_planificacion_largo_plazo();
void inicializar_proceso(t_pcb*, char*, uint32_t);
void mover_proceso_a_ready(void);
void finalizar_proceso(t_pcb*);

#endif