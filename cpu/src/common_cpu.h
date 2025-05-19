#ifndef COMMON_CPU_H_
#define COMMON_CPU_H_

#include "./utils/utils.h"

typedef struct {
    char* ip;
    char* puerto;
    int32_t id_cpu;
} datos_conexion_t;

extern char* ip_kernel;
extern char* ip_memoria;
extern char* puerto_kernel_dispatch;
extern char* puerto_kernel_interrupt;
extern char* puerto_memoria;
extern datos_conexion_t* datos_dispatch;
extern datos_conexion_t* datos_interrupt;
extern datos_conexion_t* datos_memoria;

//Configuraciones iniciales
void iniciar_cpu(int32_t);
void* conectar(void*);

//Ciclo completo de instrucción
void* ciclo_de_instruccion(int, int);

//Manejo de PCB
t_pcb* recibir_pcb(int);
t_pcb* deserializar_pcb(void*);

//Manejo de instrucción
t_list* recibir_instruccion(t_pcb*, int)
void trabajar_instruccion (t_instruccion, t_pcb*)

//Ejecución de instrucción
void ejecutar_noop();
void ejecutar_read(uint32_t,uint32_t);
void ejecutar_write(uint32_t,uint32_t);
void ejecutar_io(uint32_t,uint32_t);
void ejecutar_init_proc(uint32_t,uint32_t);
void ejecutar_dump_memory();
void ejecutar_exit();

#endif /* COMMON_CPU_H_ */
