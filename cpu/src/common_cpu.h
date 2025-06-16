#ifndef COMMON_CPU_H_
#define COMMON_CPU_H_

#include "./utils/utils.h"

typedef struct {
  char* ip;
  char* puerto;
  int32_t id_cpu;
  int socket;
} datos_conexion_t;

typedef enum {
  EJECUCION_CONTINUA,
  EJECUCION_CONTINUA_INIT_PROC,
  EJECUCION_FINALIZADA,
  EJECUCION_BLOQUEADA_IO,
  EJECUCION_BLOQUEADA_DUMP,
  EJECUCION_BLOQUEADA_SOLICITUD
} t_estado_ejecucion;

typedef struct {
  uint32_t pid;
  int pagina;
  int marco;
  uint32_t tiempo_transcurrido; //Para LRU
} estructura_tlb;

typedef struct {
  estructura_tlb* entradas;
  int cantidad_entradas;
  char* algoritmo_reemplazo;
  int reemplazo_actual; //Para FIFO
} tlb_t;

extern char* ip_kernel;
extern char* ip_memoria;
extern char* puerto_kernel_dispatch;
extern char* puerto_kernel_interrupt;
extern char* puerto_memoria;
extern int conexion_kernel_dispatch;
extern int conexion_kernel_interrupt;
extern int conexion_memoria;
extern datos_conexion_t* datos_dispatch;
extern datos_conexion_t* datos_interrupt;
extern datos_conexion_t* datos_memoria;
extern uint32_t tamanio_pagina;
extern uint32_t cant_entradas_tabla;
extern uint32_t cant_niveles;

//Configuraciones iniciales
void iniciar_cpu(int32_t);
void* conectar_dispatch(void*);
void* conectar_interrupt(void*);
void* conectar_memoria(void*);
void recibir_datos_memoria();

//Ciclo completo de instrucción
void* ciclo_de_instruccion(t_pcb*, int, int,int);

//Manejo de PCB
t_pcb* recibir_pcb(int);
t_pcb* deserializar_pcb(void*);

//Manejo de instrucción
t_list* recibir_instruccion(t_pcb*, int);
t_estado_ejecucion trabajar_instruccion (t_instruccion, t_pcb*);
void actualizar_kernel(t_instruccion,t_estado_ejecucion, t_pcb*,int);
bool chequear_interrupcion(int, uint32_t);

//Ejecución de instrucción;
void ejecutar_read(uint32_t,uint32_t,uint32_t);
void ejecutar_write(uint32_t,uint32_t,uint32_t); 
//void ejecutar_init_proc(uint32_t, uint32_t); Por ahora no la estamos usando

//Envío de actualizaciónes a Kernel
void agregar_syscall_a_paquete(t_paquete*, uint32_t, uint32_t, char*, char*, uint32_t);
void llenar_paquete (t_paquete*,t_pcb*);

//Traducción de dirección física
uint32_t traducir_direccion(uint32_t, uint32_t, uint32_t); 

//Manejo de traducciones
uint32_t consultar_cache (uint32_t, uint32_t);
uint32_t consultar_TLB (uint32_t, uint32_t);
uint32_t consultar_memoria (uint32_t, uint32_t);
//void actualizar_TLB (uint32_t,int, int); Todavía no implementada

#endif /* COMMON_CPU_H_ */
