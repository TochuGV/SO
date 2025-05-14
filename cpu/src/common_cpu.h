#ifndef COMMON_CPU_H_
#define COMMON_CPU_H_

#include "./utils/utils.h"

void* atender_cpu_dispatch(void* arg);
void* atender_cpu_interrupt(void* arg);
void* atender_memoria(void* arg);

void* conectar_kernel_dispatch(void* arg);
void* conectar_kernel_interrupt(void* arg);
void* conectar_memoria(void* arg);

int32_t handshake_kernel_dispatch(int32_t, int);
int32_t handshake_kernel_interrupt(int32_t, int);
int32_t handshake_memoria(int32_t, int);

void* manejar_dispatch(int, int);
t_pcb* recibir_pcb(int);
t_pcb* deserializar_pcb(void*);
void* recibir_instruccion(t_pcb*, int);
//void* ciclo_de_instruccion(pcb);

#endif /* COMMON_CPU_H_ */
