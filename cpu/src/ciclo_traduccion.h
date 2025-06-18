#ifndef CICLO_TRADUCCION_H_
#define CICLO_TRADUCCION_H_

#include "./utils/utils.h"
#include "conexiones.h"
#include <math.h>

typedef struct {
uint32_t pid;
int pagina;
char* contenido;
bool bit_uso; //Para Clock y Clock M
bool bit_modificado; //Para Clock M
} entrada_cache;

typedef struct {
entrada_cache* entradas;
int cantidad_entradas;
t_algoritmo_tlb algoritmo_reemplazo;
int puntero_reemplazo; 
} cache_paginas_t;

typedef struct {
  uint32_t pid;
  int pagina;
  int marco;
  uint32_t tiempo_transcurrido; //Reemplaza el mayor en LRU
  uint32_t nro_orden; //Reemplaza el menor en FIFO
} entrada_tlb;

typedef struct {
  entrada_tlb* entradas;
  int cantidad_entradas;
  t_algoritmo_tlb algoritmo_reemplazo;
} tlb_t;

extern entrada_cache* cache;
extern cache_paginas_t* parametros_cache;
extern entrada_tlb* tlb;
extern tlb_t* parametros_tlb;
extern uint32_t tamanio_pagina;
extern uint32_t cant_entradas_tabla;
extern uint32_t cant_niveles;

//Manejo de caché de Páginas
char* consultar_cache(uint32_t, uint32_t);
bool pagina_esta_en_cache(uint32_t,uint32_t);
void actualizar_cache(uint32_t,uint32_t,char*);

//Traducción de dirección física
uint32_t traducir_direccion(uint32_t, uint32_t, uint32_t); 

//Manejo de traducciones
uint32_t consultar_TLB (uint32_t, uint32_t);
uint32_t consultar_memoria (uint32_t, uint32_t);
void actualizar_TLB (uint32_t,uint32_t, uint32_t);

//Auxiliares para asignaciones en Cache y TLB
void asignar_lugar_en_TLB(int,uint32_t,uint32_t,uint32_t);

//Auxiliares para ejecutar Write y Read
char* pedir_valor_a_memoria(uint32_t, char*);
void escribir_valor_en_memoria(uint32_t, char*);

//Auxiliar de conversión
uint32_t convertir_a_uint32_t (char*);

#endif /* CICLO_TRADUCCION_H_ */