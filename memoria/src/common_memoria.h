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


// MEMORIA USUARIO
extern uint32_t tamanio_memoria;
extern void* memoria;
extern uint32_t memoria_usada;
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


////////////////////////////////////////// FUNCIONES //////////////////////////////////////////

// CONEXIONES
void* atender_cliente(void* arg);
int recibir_handshake_memoria(int cliente_memoria);

// UTILS
void inicializar_memoria(void);
void terminar_memoria(void);

#endif /* COMMON_MEMORIA_H_ */