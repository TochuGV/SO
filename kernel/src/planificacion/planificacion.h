#ifndef PLANIFICACION_H_
#define PLANIFICACION_H_

#include "pcb/pcb.h"
#include "utils/utils.h" // Revisar después
#include <commons/collections/queue.h>
#include <semaphore.h>

extern t_queue* cola_new;
extern t_queue* cola_ready;
extern pthread_mutex_t mutex_new;       // Mutex para proteger la cola NEW
extern pthread_mutex_t mutex_ready;     // Mutex para proteger la cola READY
extern sem_t hay_procesos_en_new;       // Semáforo para indicar si hay procesos en NEW

//                                                                     COLA DE READY
// Declaramos funciones
// Planificación de largo plazo
void iniciar_planificacion_largo_plazo();
void inicializar_proceso(t_pcb*);
void cambiar_estado(t_pcb*, t_estado, t_estado);
void finalizar_proceso(t_pcb*);

// Planificación de corto plazo (READY)
void iniciar_planificacion_corto_plazo();
void mover_proceso_a_ready(t_pcb*);
t_pcb* obtener_siguiente_proceso();

#endif