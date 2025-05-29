#ifndef PLANIFICACION_H_
#define PLANIFICACION_H_

#include "pcb/pcb.h"
#include "logger/logger.h"
#include "common/common.h"
#include "utils/utils.h" // Revisar después
#include <commons/collections/queue.h>
#include <semaphore.h>

extern t_queue* cola_new;
extern t_queue* cola_ready;

extern pthread_mutex_t mutex_new;       // Mutex para proteger la cola NEW
extern pthread_mutex_t mutex_ready;     // Mutex para proteger la cola READY
extern pthread_mutex_t mutex_cpus;

extern sem_t hay_procesos_en_new;       // Semáforo para indicar si hay procesos en NEW

// Declaramos funciones
// Planificación de largo plazo
void iniciar_planificacion_largo_plazo();
void iniciar_planificacion_corto_plazo();
void entrar_estado(t_pcb*, int);
void inicializar_proceso(t_pcb*);
void cambiar_estado(t_pcb*, t_estado, t_estado);
void finalizar_proceso(t_pcb*);

// Planificación de corto plazo (READY)
//void mover_proceso_a_ready(char*, int32_t);
t_pcb* mover_proceso_a_exec();

#endif

