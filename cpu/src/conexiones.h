#ifndef CONEXIONES_H
#define CONEXIONES_H

#include "./utils/utils.h"
#include "ciclo_instruccion.h"
#include "ciclo_traduccion.h"

typedef struct {
  char* ip;
  char* puerto;
  int32_t id_cpu;
  int socket;
} datos_conexion_t;

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

//auxiares para TLB y caché
t_algoritmo_cache convertir_cache(char*);
t_algoritmo_tlb convertir_tlb(char*);

#endif /* CONEXIONES_H */