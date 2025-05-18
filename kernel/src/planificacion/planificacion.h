#ifndef PLANIFICACION_H_
#define PLANIFICACION_H_

#include "pcb/pcb.h"
#include "utils/utils.h" // Revisar después
#include <commons/collections/queue.h>
//#include <semaphore.h>

//extern t_queue* cola_new;
//extern t_queue* cola_ready;
//extern sem_t hay_procesos_en_new;

void iniciar_planificacion_largo_plazo();
void agregar_proceso_a_new(t_pcb*);
void finalizar_proceso(t_pcb*);

#endif