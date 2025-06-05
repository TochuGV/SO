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

#endif /* ATENCION_KERNEL_H_ */