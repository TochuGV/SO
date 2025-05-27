#ifndef COMMON_H_
#define COMMON_H_

#include "./utils/utils.h"
#include "pcb/pcb.h"
#include <stdint.h>

extern int conexion_cpu_dispatch;
extern int conexion_cpu_interrupt;
extern int conexion_io;
extern int conexion_memoria;

void* conectar_cpu_dispatch(void* arg);
void* conectar_io(void* arg);
void* atender_cpu_dispatch(void* socket_cpu_dispatch);
void* atender_io(void* socket_io);
int recibir_handshake_kernel(int);
void* atender_cliente(void* arg);
uint32_t enviar_proceso_a_memoria(char*, uint32_t, uint32_t, int);
int32_t handshake_kernel(int conexion_memoria);
char* crear_cadena_metricas_estado(t_pcb*);

extern char* NOMBRES_SYSCALLS[4];
extern char* NOMBRES_DISPOSITIVOS_IO[5];

char* token_io_to_string(int32_t);
#endif /* COMMON_H_ */