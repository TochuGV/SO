#ifndef COMMON_CPU_H_
#define COMMON_CPU_H_

#include "./utils/utils.h"

int32_t handshake_cpu(int, int);

void* manejar_dispatch(void* arg):
t_pcb* recibir_pcb(int puerto_kernel_dispatch);
t_pcb* deserializar_pcb(void* buffer);
void* ciclo_de_instruccion(pcb);

#endif /* COMMON_CPU_H_ */