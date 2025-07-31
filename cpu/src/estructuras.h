#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
  const char* ip;
  const char* puerto;
  int32_t id_cpu;
  int socket;
} datos_conexion_t;

typedef enum {
  LRU,
  FIFO
} t_algoritmo_tlb;

typedef enum {
  CLOCK,
  CLOCK_M
} t_algoritmo_cache;

typedef struct {
uint32_t pid;
int pagina;
char* contenido;
bool bit_uso; //Para Clock y Clock M
bool bit_modificado; //Para Clock M
uint32_t desplazamiento;
} entrada_cache;

typedef struct {
entrada_cache* entradas;
int cantidad_entradas;
t_algoritmo_tlb algoritmo_reemplazo;
int puntero_reemplazo; 
} cache_paginas_t;

typedef struct {
  uint32_t pid;
  int pagina;
  int marco;
  uint32_t tiempo_transcurrido; //Reemplaza el mayor en LRU
  uint32_t nro_orden; //Reemplaza el menor en FIFO
} entrada_tlb;

typedef struct {
  entrada_tlb* entradas;
  int cantidad_entradas;
  t_algoritmo_tlb algoritmo_reemplazo;
} tlb_t;

typedef struct {
  //Identificador de la CPU
  int id;

  //Socket de las conexiones
  int conexion_kernel_dispatch;
  int conexion_kernel_interrupt;
  int conexion_memoria;

  //Todos los datos para las conexiones
  datos_conexion_t* datos_dispatch;
  datos_conexion_t* datos_interrupt;
  datos_conexion_t* datos_memoria;

  //Caché y TLB
  entrada_cache* cache;
  cache_paginas_t* parametros_cache;
  uint32_t retardo_cache;
  entrada_tlb* tlb;
  tlb_t* parametros_tlb;
  uint32_t orden_actual_tlb;
} t_cpu;

extern const char* ip_kernel;
extern const char* ip_memoria;
extern const char* puerto_kernel_dispatch;
extern const char* puerto_kernel_interrupt;
extern const char* puerto_memoria;
extern uint32_t tamanio_pagina;
extern uint32_t cant_entradas_tabla;
extern uint32_t cant_niveles;

#endif /* ESTRUCTURAS_H */