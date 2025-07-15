#ifndef SJF_H
#define SJF_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include "utils/utils.h"
#include "init/init.h"
#include "planificacion/planificadores/corto_plazo/algoritmos/algoritmos.h"

// Selección y planificación
t_pcb* seleccionar_proceso_con_menor_estimacion(t_queue*);
t_pcb* obtener_proximo_proceso_ready(t_queue*);
void reordenar_cola_ready_por_estimacion(t_queue* cola_ready);
bool comparar_estimaciones(t_pcb* pcb1, t_pcb* pcb2);

// Estimaciones
void inicializar_estimacion_rafaga(uint32_t);
double obtener_estimacion(uint32_t);
void actualizar_estimacion(uint32_t, double);

// Limpieza
void destruir_diccionario_estimaciones(void);
void mover_proceso_a_exec_sjf(void);

#endif /* SJF_H */
