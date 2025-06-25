#ifndef CICLO_TRADUCCION_H_
#define CICLO_TRADUCCION_H_

#include "./utils/utils.h"
#include "estructuras.h"
#include <math.h>

//Manejo de caché de Páginas
char* consultar_contenido_cache(t_cpu*,uint32_t, uint32_t);
int consultar_pagina_cache(t_cpu*,uint32_t, uint32_t);
void actualizar_cache(t_cpu*,uint32_t,uint32_t,char*,bool);
void finalizar_proceso_en_cache(t_cpu*,uint32_t,t_estado_ejecucion);

//Traducción de dirección física
uint32_t traducir_direccion(t_cpu*,uint32_t,uint32_t,uint32_t); 

//Manejo de traducciones
uint32_t consultar_TLB (t_cpu*,uint32_t,uint32_t);
uint32_t consultar_memoria (t_cpu*,uint32_t,uint32_t);
void actualizar_TLB (t_cpu*,uint32_t,uint32_t,uint32_t);

//Auxiliares para asignaciones en Cache y TLB
void asignar_lugar_en_cache(t_cpu*,int,uint32_t,uint32_t,char*,bool);
void asignar_lugar_en_TLB(t_cpu*,int,uint32_t,uint32_t,uint32_t,int);

//Auxiliares para ejecutar Write y Read
char* pedir_valor_a_memoria(t_cpu*,uint32_t,uint32_t,char*);
void escribir_valor_en_memoria(t_cpu*,uint32_t,uint32_t,char*);

#endif /* CICLO_TRADUCCION_H_ */