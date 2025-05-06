#ifndef COMMON_MEMORIA_H_
#define COMMON_MEMORIA_H_

#include "./utils/utils.h"
#include <stdint.h>

////////////////////////////////////////// VARIABLES EXTERNAS //////////////////////////////////////////

// CONEXIONES
extern char* puerto_escucha;
extern int conexion_kernel;
//extern int conexion_cpu;

// MEMORIA USUARIO
extern uint32_t tamanio_memoria;
extern void* memoria;
extern uint32_t memoria_usada;


////////////////////////////////////////// FUNCIONES //////////////////////////////////////////

// CONEXIONES
void inicializar_memoria(void);
void* atender_kernel(void*);
void* conectar_kernel(void*);

// KERNEL
bool verificar_espacio_memoria(uint32_t tamanio_proceso);
void* recibir_y_ubicar_proceso(void);
t_list* leer_archivo_instrucciones(char* path);
void* ubicar_proceso_en_memoria(int tamanio_proceso, t_list* lista_instrucciones);

void terminar_memoria(void);

#endif /* COMMON_MEMORIA_H_ */