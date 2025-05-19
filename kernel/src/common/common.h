#ifndef COMMON_H_
#define COMMON_H_

#include "./utils/utils.h"
#include "pcb/pcb.h"
#include <stdint.h>

extern int conexion_cpu_dispatch;
extern int conexion_cpu_interrupt;
extern int conexion_io;

void* atender_cpu_dispatch(void* socket_cpu_dispatch);
void* atender_io(void* socket_io);
void* conectar_cpu_dispatch(void* arg);
void* conectar_io(void* arg);
void* enviar_proceso_a_memoria(char* path, uint32_t tamanio_proceso, int socket_cliente);
int32_t handshake_kernel(int conexion_memoria);
char* crear_cadena_metricas_estado(t_pcb*);

#endif /* COMMON_H_ */