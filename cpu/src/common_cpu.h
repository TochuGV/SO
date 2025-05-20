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
void actualizar_kernel(t_instruccion,t_estado_ejecucion,t_pcb*,int);

//Ejecución de instrucción;
void ejecutar_read(uint32_t,uint32_t);
void ejecutar_write(uint32_t,uint32_t);

//Envío de actualizaciónes a Kernel
void enviar_finalizacion(t_instruccion,t_estado_ejecucion,t_pcb*,int);
void enviar_bloqueo_IO(t_instruccion,t_estado_ejecucion,t_pcb*,int);
void enviar_bloqueo_INIT_PROC(t_instruccion,t_estado_ejecucion,t_pcb*,int);
void enviar_bloqueo_DUMP(t_instruccion,t_estado_ejecucion,t_pcb*,int);
void llenar_paquete (t_paquete*,t_estado_ejecucion,t_pcb*);


#endif /* COMMON_CPU_H_ */
