#ifndef CONEXIONES_H
#define CONEXIONES_H

#include "estructuras.h"
#include "./utils/utils.h"

//Configuraciones iniciales
t_cpu* iniciar_cpu(int32_t);
void* conectar_dispatch(void*);
void* conectar_interrupt(void*);
void* conectar_memoria(void*);
void recibir_datos_memoria(t_cpu*);

//auxiares para TLB y caché
t_algoritmo_cache convertir_cache(char*);
t_algoritmo_tlb convertir_tlb(char*);

//auxiliar para liberar memoria
void liberar_cpu(t_cpu*);

#endif /* CONEXIONES_H */