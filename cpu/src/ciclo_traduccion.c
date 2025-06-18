#include "ciclo_traduccion.h"

entrada_cache* cache;
cache_paginas_t* parametros_cache;
entrada_tlb* tlb;
tlb_t* parametros_tlb;
uint32_t tamanio_pagina;
uint32_t cant_entradas_tabla;
uint32_t cant_niveles;

//Caché de páginas
//verificar si tengo el valor que necesito leer
char* consultar_cache (uint32_t pid, uint32_t nro_pagina) {
  for (int i=0;i<parametros_cache->cantidad_entradas ;i++) {
    if (cache[i].pid==pid && cache[i].pagina==nro_pagina) {
        //Log 8. Página encontrada en Caché
        log_info(logger, "PID: %d - Cache Hit - Pagina: %d", pid, nro_pagina);
        return cache[i].contenido;
    }
  }
  //Log 7. Página faltante en Caché
  log_info(logger, "PID: %d - Cache Miss - Pagina: %d", pid, nro_pagina);
  return NULL;
}

//Verificar si ya tengo la página que busco escribir 
bool pagina_esta_en_cache(uint32_t pid, uint32_t nro_pagina) {
    for (int i=0;i<parametros_cache->cantidad_entradas ;i++) {
    if (cache[i].pid==pid && cache[i].pagina==nro_pagina) {
        //Log 8. Página encontrada en Caché
        log_info(logger, "PID: %d - Cache Hit - Pagina: %d", pid, nro_pagina);
        return 1;
    }
  }
  //Log 7. Página faltante en Caché
  log_info(logger, "PID: %d - Cache Miss - Pagina: %d", pid, nro_pagina);
  return 0;
}

//
void actualizar_cache(uint32_t pid,uint32_t nro_pagina,char* valor_a_escribir) {
  return;
}

//MMU
uint32_t traducir_direccion(uint32_t pid, uint32_t nro_pagina,uint32_t desplazamiento) {

  uint32_t marco = -1;

  if (parametros_tlb->cantidad_entradas>0) {
    marco = consultar_TLB(pid, nro_pagina);
  }
  
  if(marco ==-1){
    marco = consultar_memoria(pid, nro_pagina);

    if(marco == -1){
      log_error(logger, "No se pudo obtener el marco para la página %d del PID %d", nro_pagina, pid);
      return -1;
    };
    
    actualizar_TLB(pid, nro_pagina, marco);
    }
  //Log 5. Obtener Marco
  log_info(logger, "PID: %d - OBTENER MARCO - Página: %d - Marco: %d", pid, nro_pagina, marco);
  return marco * tamanio_pagina + desplazamiento;
};

//Consultar TLB si falló en Caché
uint32_t consultar_TLB (uint32_t pid, uint32_t nro_pagina) {
  for (int i=0;i<parametros_tlb->cantidad_entradas ;i++) {
    tlb[i].tiempo_transcurrido++;
    if (tlb[i].pid==pid && tlb[i].pagina==nro_pagina) {
        tlb[i].nro_orden = i;
        //Log 6. TLB Hit
        log_info(logger, "PID: %d - TLB HIT - Pagina: %d", pid, nro_pagina);
        return tlb[i].marco;
    }
  } 
  //Log 7. TLB Miss
  log_info(logger, "PID: %d - TLB MISS - Pagina: %d", pid, nro_pagina);
  return -1;
}

//Consultar memoria si falló en TLB
uint32_t consultar_memoria(uint32_t pid, uint32_t nro_pagina) {

  uint32_t marco;
  t_paquete* paquete = crear_paquete(SOLICITUD_MARCO);
  agregar_a_paquete(paquete, &pid, sizeof(uint32_t)); 

  for (int nivel=1; nivel <= cant_niveles; nivel++) {
    uint32_t entrada_nivel = (uint32_t) floor(nro_pagina / pow(cant_entradas_tabla,cant_niveles - nivel)) % cant_entradas_tabla;
    agregar_a_paquete(paquete, &entrada_nivel, sizeof(uint32_t)); 
  }

  enviar_paquete(paquete, conexion_memoria);
  recv(conexion_memoria, &marco, sizeof(uint32_t), MSG_WAITALL);
  return marco;
}


//Insertar un nuevo marco en el TLB
void actualizar_TLB (uint32_t pid, uint32_t pagina, uint32_t marco) {
  int index_orden = 10000;
  int index_tiempo = -1;
  int reemplazo;
  
  //busco entrada libre
  for(int i= 0; i < parametros_tlb -> cantidad_entradas; i++){
    if(tlb[i].pid == -1){
      asignar_lugar_en_TLB(i,pid,marco,pagina);
    return;
  }

  switch (parametros_tlb->algoritmo_reemplazo){
    case LRU:
    for (int i=0; i < parametros_tlb -> cantidad_entradas; i++){
      if(tlb[i].tiempo_transcurrido > index_tiempo) {
        reemplazo = i;
      }
      asignar_lugar_en_TLB(reemplazo,pid,marco,pagina);
    }
    break;

    case FIFO:
    for (int i=0;i <= parametros_tlb->cantidad_entradas;i++) {
      if (tlb[i].nro_orden < index_orden) {
        reemplazo = i;
      }
    }
    asignar_lugar_en_TLB(reemplazo,pid,marco,pagina);
    break;
    }
  }
}
//Auxiliar para hacer los reemplazos y asignaciones en el TLB
void asignar_lugar_en_TLB(int ubicacion, uint32_t pid, uint32_t marco, uint32_t pagina) {
  tlb[ubicacion].pid = pid;
  tlb[ubicacion].pagina = pagina;
  tlb[ubicacion].marco = marco;
  tlb[ubicacion].nro_orden = ubicacion;
  tlb[ubicacion].tiempo_transcurrido = 0;
}

char* pedir_valor_a_memoria(uint32_t direccion_fisica, char* tamaño){
  t_paquete* paquete = crear_paquete(LECTURA);

  agregar_a_paquete(paquete, &direccion_fisica, sizeof(uint32_t));
  agregar_a_paquete(paquete, &tamaño, sizeof(char*));

  enviar_paquete(paquete, conexion_memoria);

  char* valor;

  recv(conexion_memoria, &valor, sizeof(char*), MSG_WAITALL);

  return valor;
}

void escribir_valor_en_memoria(uint32_t direccion_fisica, char* valor){
  uint32_t valor_a_escribir = convertir_a_uint32_t(valor);

  t_paquete* paquete = crear_paquete(ESCRITURA);

  agregar_a_paquete(paquete, &direccion_fisica, sizeof(uint32_t));
  agregar_a_paquete(paquete, &valor_a_escribir, sizeof(uint32_t));

  enviar_paquete(paquete, conexion_memoria);
};

//Función auxiliar para pasar de Char* a uint32_t
uint32_t convertir_a_uint32_t (char* valor) {
    char* endptr;
    
    unsigned long temp = strtoul(valor, &endptr, 10);

    return (uint32_t)temp;
}