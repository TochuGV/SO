#ifndef COMMON_MEMORIA_H_
#define COMMON_MEMORIA_H_

#include "./utils/utils.h"
#include <stdint.h>

////////////////////////////////////////// VARIABLES EXTERNAS //////////////////////////////////////////
#define CANTIDAD_INSTRUCCIONES 8

// CONEXIONES
extern char* puerto_escucha;
extern int servidor_memoria;


// MEMORIA USUARIO
extern uint32_t tamanio_memoria;
extern void* memoria;
extern uint32_t memoria_usada;
extern char* path_instrucciones;
extern t_list* lista_procesos;
extern char* NOMBRES_INSTRUCCIONES[CANTIDAD_INSTRUCCIONES];


////////////////////////////////////////// ESTRUCTURAS //////////////////////////////////////////

// CPU
typedef struct
{
  uint32_t pid;
  t_list* lista_instrucciones;
} t_proceso;



////////////////////////////////////////// FUNCIONES //////////////////////////////////////////

// CONEXIONES
void* atender_cliente(void* arg);
int recibir_handshake_memoria(int cliente_memoria);

// KERNEL
void* atender_kernel(void*);
bool verificar_espacio_memoria(uint32_t tamanio_proceso);
int recibir_y_ubicar_proceso(int);
t_list* leer_archivo_instrucciones(char* path);

// CPU
void* atender_cpu(void*);
void* recibir_solicitud_instruccion(int cliente_cpu);

// UTILS
void inicializar_memoria(void);
void terminar_memoria(void);

#endif /* COMMON_MEMORIA_H_ */