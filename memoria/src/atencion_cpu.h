#ifndef ATENCION_CPU_H_
#define ATENCION_CPU_H_

#include "./utils/utils.h"
#include "common_memoria.h"
#include <stdint.h>



////////////////////////////////////////// VARIABLES EXTERNAS //////////////////////////////////////////



////////////////////////////////////////// ESTRUCTURAS //////////////////////////////////////////



////////////////////////////////////////// FUNCIONES //////////////////////////////////////////

void* atender_cpu(void*);
void* recibir_solicitud_instruccion(int cliente_cpu);
void recibir_solicitud_marco(int cliente_cpu);
void recibir_solicitud_lectura(int cliente_cpu);
void recibir_solicitud_escritura(int cliente_cpu);

#endif /* ATENCION_CPU_H_ */