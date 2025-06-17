#ifndef ATENCION_KERNEL_H_
#define ATENCION_KERNEL_H_

#include "./utils/utils.h"
#include "common_memoria.h"
#include <stdint.h>


////////////////////////////////////////// VARIABLES EXTERNAS //////////////////////////////////////////


////////////////////////////////////////// ESTRUCTURAS //////////////////////////////////////////


////////////////////////////////////////// FUNCIONES //////////////////////////////////////////

void* atender_kernel(void*);
bool verificar_espacio_memoria(uint32_t tamanio_proceso);
int recibir_y_ubicar_proceso(int);
t_list* leer_archivo_instrucciones(char* path);
int finalizar_proceso(uint32_t pid);
int atender_dump_memory(uint32_t pid);
void escribir_dump(t_tabla* tabla_de_paginas, FILE* file);

#endif /* ATENCION_KERNEL_H_ */