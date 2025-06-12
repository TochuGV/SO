#ifndef COMMON_MEMORIA_H_
#define COMMON_MEMORIA_H_

#include "./utils/utils.h"
#include "atencion_cpu.h"
#include "atencion_kernel.h"
#include <stdint.h>

#define CANTIDAD_INSTRUCCIONES 8


////////////////////////////////////////// VARIABLES EXTERNAS //////////////////////////////////////////

// CONEXIONES
extern char* puerto_escucha;
extern int servidor_memoria;


// VALORES DE MEMORIA
extern void* memoria;
extern uint32_t tamanio_memoria;
extern uint32_t memoria_usada;
extern uint32_t tamanio_pagina;
extern uint32_t entradas_por_tabla;
extern uint32_t cantidad_niveles;
extern uint32_t retardo_memoria;
extern uint32_t retardo_swap;
extern char* path_swapfile;
extern char* path_dump;
extern char* path_instrucciones;


// LISTAS
extern t_list* lista_procesos;
extern t_list* lista_ids_cpus;


// INSTRUCCIONES
extern char* NOMBRES_INSTRUCCIONES[CANTIDAD_INSTRUCCIONES];

////////////////////////////////////////// ESTRUCTURAS //////////////////////////////////////////

typedef struct
{
  uint32_t pid;
  t_list* lista_instrucciones;
} t_proceso;

typedef struct
{
  int32_t id;
  int socket;
} t_cpu_id_socket;

typedef struct 
{
  uint32_t nivel;
  t_list* entradas;
} t_tabla;

typedef struct
{
  uint32_t marco;
  t_tabla* siguiente_tabla;
} t_entrada;

////////////////////////////////////////// FUNCIONES //////////////////////////////////////////

// CONEXIONES
void* atender_cliente(void* arg);
int recibir_handshake_memoria(int cliente_memoria);

// UTILS
void inicializar_memoria(void);
void obtener_configs(void);
void terminar_memoria(void);

// PAGINAS
t_tabla* crear_tabla_multinivel(void);
t_tabla* crear_tablas_multinivel(uint32_t nivel_actual);

#endif /* COMMON_MEMORIA_H_ */