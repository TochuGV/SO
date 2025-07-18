#ifndef MEDIANO_PLAZO_H_
#define MEDIANO_PLAZO_H_

#include "utils/utils.h"
#include "pcb/pcb.h"
#include "common/common.h"
#include <commons/collections/queue.h>
#include <semaphore.h>
#include "planificacion/planificadores/largo_plazo/largo_plazo.h"

extern t_list* lista_susp_blocked;
extern t_queue* cola_susp_ready;
extern sem_t semaforo_revisar_susp_ready;
extern pthread_mutex_t mutex_susp_blocked;
extern pthread_mutex_t mutex_susp_ready;
extern pthread_mutex_t mutex_info_mediano_plazo;
extern t_list* lista_info_procesos_susp;

typedef struct {
  uint32_t pid;
  uint32_t tamanio;
} t_informacion_mediano_plazo;

void iniciar_planificacion_mediano_plazo(void);
void* revisar_bloqueados(void*);
void suspender_proceso(t_pcb*);
int esta_suspendido(t_pcb*);
void encolar_proceso_en_susp_ready(t_pcb*);
void desuspender_proceso(void);
void reordenar_cola_susp_ready_pmcp(t_queue* cola_susp_ready);
bool comparar_tamanios(t_pcb* pcb1, t_pcb* pcb2);
uint32_t obtener_tamanio(uint32_t pid);

#endif /* MEDIANO_PLAZO_H */