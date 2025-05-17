#ifndef COMMON_CPU_H_
#define COMMON_CPU_H_

#include "./utils/utils.h"

typedef struct {
    char* ip;
    char* puerto;
} datos_conexion_t;

extern char* ip_kernel;
extern char* ip_memoria;
extern char* puerto_kernel_dispatch;
extern char* puerto_kernel_interrupt;
extern char* puerto_memoria;
extern datos_conexion_t* datos_dispatch;
extern datos_conexion_t* datos_interrupt;
extern datos_conexion_t* datos_memoria;

void iniciar_cpu();
void* conectar(void*);

void* manejar_dispatch(int, int);
t_pcb* recibir_pcb(int);
t_pcb* deserializar_pcb(void*);
void* recibir_instruccion(t_pcb*, int);
//void* ciclo_de_instruccion(pcb);

void decode(t_instruccion instruccion, uint32_t pid);

#endif /* COMMON_CPU_H_ */
