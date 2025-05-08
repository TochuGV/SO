#ifndef COMMON_CPU_H_
#define COMMON_CPU_H_

#include "./utils/utils.h"

typedef struct {
    char* ip_kernel;
    char* ip_memoria;
    char* puerto_kernel_dispatch;
    char* puerto_kernel_interrupt;
    char* puerto_memoria;
    int socket_dispatch;
    int socket_interrupt;
    int socket_memoria;
} t_cpu_args;

void* conexiones_modulos(void* arg);

int32_t handshake_cpu(int, int);
int32_t handshake_memoria(int32_t, int);

void* manejar_dispatch(t_cpu_args*);
t_pcb* recibir_pcb(int);
t_pcb* deserializar_pcb(void*);
void* serializar_pcb(t_pcb*);
void* enviar_pc_a_memoria(t_pcb*, int);
//void* ciclo_de_instruccion(pcb);

#endif /* COMMON_CPU_H_ */