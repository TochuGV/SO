#ifndef COMMON_MEMORIA_H_
#define COMMON_MEMORIA_H_

#include "./utils/utils.h"
#include <stdint.h>
#include <commons/temporal.h>

#define CANTIDAD_INSTRUCCIONES 8
#define CANTIDAD_METRICAS 6


////////////////////////////////////////// VARIABLES EXTERNAS //////////////////////////////////////////

// CONEXIONES
extern char* puerto_escucha;
extern int servidor_memoria;


// VALORES DE MEMORIA
extern void* memoria;
extern uint32_t tamanio_memoria;
extern uint32_t tamanio_pagina;
extern uint32_t entradas_por_tabla;
extern uint32_t cantidad_niveles;
extern uint32_t retardo_memoria;
extern uint32_t retardo_swap;
extern char* path_swapfile;
extern char* path_dump;
extern char* path_instrucciones;
extern uint32_t cantidad_marcos;
extern uint32_t marcos_libres;
extern uint8_t* bitmap_marcos;
extern FILE* swapfile;
extern uint32_t swap_offset;
extern pthread_mutex_t mutex_memoria;
extern pthread_mutex_t mutex_swapfile;
extern pthread_mutex_t mutex_swap_offset;
extern pthread_mutex_t mutex_marcos_libres;
extern pthread_mutex_t mutex_bitmap;

// LISTAS
extern t_list* lista_procesos;
extern t_list* lista_ids_cpus;


// INSTRUCCIONES
extern char* NOMBRES_INSTRUCCIONES[CANTIDAD_INSTRUCCIONES];

////////////////////////////////////////// ESTRUCTURAS //////////////////////////////////////////

typedef struct 
{
  uint32_t nivel;
  t_list* entradas;
} t_tabla;

typedef struct
{
  uint32_t pid;
  t_list* lista_instrucciones;
  t_tabla* tabla_de_paginas;
  uint32_t marcos_en_uso;
  uint32_t metricas[CANTIDAD_METRICAS];
  int32_t base_swap;
  int32_t tamanio_swap;
} t_proceso;

typedef struct
{
  int32_t id;
  int socket;
} t_cpu_id_socket;

typedef struct
{
  int32_t marco;
  t_tabla* siguiente_tabla;
} t_entrada;

typedef enum {
  ACCESOS_TABLA_PAGINAS,
  INSTRUCCIONES_SOLICITADAS,
  BAJADAS_A_SWAP,
  SUBIDAS_A_MP,
  LECTURAS,
  ESCRITURAS
} t_metricas_memoria;

////////////////////////////////////////// FUNCIONES //////////////////////////////////////////

// CONEXIONES
void* atender_cliente(void* arg);
int recibir_handshake_memoria(int cliente_memoria);

// UTILS
void inicializar_memoria(void);
void obtener_configs(void);
void terminar_memoria(void);

// PROCESOS
t_proceso* obtener_proceso(uint32_t pid);

// PAGINAS
t_tabla* crear_tabla_multinivel(uint32_t*);
t_tabla* crear_tablas_multinivel(uint32_t, uint32_t*);
uint32_t asignar_marco_libre(void);
void liberar_marcos(t_tabla*);

#endif /* COMMON_MEMORIA_H_ */