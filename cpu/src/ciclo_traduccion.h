#ifndef CICLO_TRADUCCION_H_
#define CICLO_TRADUCCION_H_

#include "./utils/utils.h"
#include "conexiones.h"
#include <math.h>

typedef struct {
  uint32_t pid;
  int pagina;
  int marco;
  uint32_t tiempo_transcurrido; //Para LRU
} estructura_tlb;

typedef struct {
  estructura_tlb* entradas;
  int cantidad_entradas;
  char* algoritmo_reemplazo;
  int reemplazo_actual; //Para FIFO
} tlb_t;

extern estructura_tlb* tlb;
extern tlb_t* parametros_tlb;
extern uint32_t tamanio_pagina;
extern uint32_t cant_entradas_tabla;
extern uint32_t cant_niveles;

//Traducción de dirección física
uint32_t traducir_direccion(uint32_t, uint32_t, uint32_t); 

//Manejo de traducciones
uint32_t consultar_cache (uint32_t, uint32_t);
uint32_t consultar_TLB (uint32_t, uint32_t);
uint32_t consultar_memoria (uint32_t, uint32_t);
//void actualizar_TLB (uint32_t,int, int); Todavía no implementada

//Auxiliares para ejecutar Write y Read
void pedir_valor_a_memoria(uint32_t, uint32_t*);
void escribir_valor_en_memoria(uint32_t, uint32_t);

#endif