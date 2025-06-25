#ifndef PCB_H
#define PCB_H

#include <stdint.h>
#include <utils/utils.h>

uint32_t generar_pid();
t_pcb* crear_pcb();
void destruir_pcb(t_pcb*);
void* serializar_pcb(t_pcb*, int*);
char* obtener_nombre_estado(t_estado);

#endif