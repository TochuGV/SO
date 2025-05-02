#ifndef COMMON_KERNEL_H_
#define COMMON_KERNEL_H_

#include "./utils/utils.h"
#include <stdint.h>

extern int conexion_cpu_dispatch;
extern int conexion_cpu_interrupt;
extern int conexion_io;

t_pcb* crear_pcb();
void destruir_pcb(t_pcb* pcb);
void* serializar_pcb(t_pcb* pcb, int bytes);
void* enviar_proceso_a_memoria(char* path, uint32_t tamanio_proceso, int socket_cliente);
void* atender_cpu_dispatch(void* socket_cpu_dispatch);
void* atender_io(void* socket_io);
int32_t handshake_kernel(int conexion_memoria);
void* conectar_cpu_dispatch(void* arg);
void* conectar_io(void* arg);

#endif /* COMMON_KERNEL_H_ */