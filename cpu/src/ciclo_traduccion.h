#ifndef CICLO_TRADUCCION_H_
#define CICLO_TRADUCCION_H_

#include "./utils/utils.h"
#include "conexiones.h"
#include <math.h>


//Manejo de caché de Páginas
char* consultar_contenido_cache(uint32_t, uint32_t);
int consultar_pagina_cache(uint32_t, uint32_t);
void actualizar_cache(uint32_t,uint32_t,char*,bool);
void finalizar_proceso_en_cache(uint32_t,t_estado_ejecucion);

//Traducción de dirección física
uint32_t traducir_direccion(uint32_t, uint32_t, uint32_t); 

//Manejo de traducciones
uint32_t consultar_TLB (uint32_t, uint32_t);
uint32_t consultar_memoria (uint32_t, uint32_t);
void actualizar_TLB (uint32_t,uint32_t, uint32_t);

//Auxiliares para asignaciones en Cache y TLB
void asignar_lugar_en_cache(int, uint32_t,uint32_t,char*,bool);
void asignar_lugar_en_TLB(int,uint32_t,uint32_t,uint32_t,int);

//Auxiliares para ejecutar Write y Read
char* pedir_valor_a_memoria(uint32_t, uint32_t, char*);
void escribir_valor_en_memoria(uint32_t, uint32_t, char*);

//Auxiliar de conversión
uint32_t convertir_a_uint32_t (char*);

#endif /* CICLO_TRADUCCION_H_ */