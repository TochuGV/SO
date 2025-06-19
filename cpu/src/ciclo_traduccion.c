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
char* consultar_contenido_cache (uint32_t pid, uint32_t nro_pagina) {
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

//Verificar si tengo la página que estoy buscando escribir
int consultar_pagina_cache (uint32_t pid, uint32_t nro_pagina) {
  for (int i=0;i<parametros_cache->cantidad_entradas ;i++) {
    if (cache[i].pid==pid && cache[i].pagina==nro_pagina) {
        //Log 8. Página encontrada en Caché
        log_info(logger, "PID: %d - Cache Hit - Pagina: %d", pid, nro_pagina);
        
        return i;  
    }
  }
  //Log 7. Página faltante en Caché
  log_info(logger, "PID: %d - Cache Miss - Pagina: %d", pid, nro_pagina);
  return 0;
}

//Actualiza la caché con el nuevo valor según el algoritmo
void actualizar_cache(uint32_t pid,uint32_t nro_pagina,char* valor_a_escribir) {
  int cantidad=parametros_cache->cantidad_entradas;

  //busco entrada libre
  for(int i= 0; i < parametros_tlb -> cantidad_entradas; i++){
    if(cache[i].pid == -1){
      asignar_lugar_en_cache(i,pid,nro_pagina,valor_a_escribir);
      parametros_cache->puntero_reemplazo= (i+1) % cantidad;
    return;
    }
  }
  
  switch (parametros_cache->algoritmo_reemplazo){
    case CLOCK:
      while(true){
        if(cache[parametros_cache->puntero_reemplazo].bit_uso == 0) {
          asignar_lugar_en_cache(parametros_cache->puntero_reemplazo,pid,nro_pagina,valor_a_escribir);
          parametros_cache->puntero_reemplazo = (parametros_cache->puntero_reemplazo + 1) % cantidad;
          return;
        }
        else {
          cache[parametros_cache->puntero_reemplazo].bit_uso=0;
          parametros_cache->puntero_reemplazo = (parametros_cache->puntero_reemplazo + 1) % cantidad;
        }
      }
    break;

    case CLOCK_M:
      int vueltas = 0;
      while (vueltas < 2) {
        for (int i = 0; i < cantidad; i++) {
          int index = (parametros_cache->puntero_reemplazo + i) % cantidad;

          if(vueltas==0 && cache[index].bit_uso == 0 && cache[index].bit_modificado == 0) {
          asignar_lugar_en_cache(index,pid,nro_pagina,valor_a_escribir);
          parametros_cache->puntero_reemplazo = (index + 1) % cantidad;
          return;
          }

          if(vueltas==1 && cache[index].bit_uso == 0 && cache[index].bit_modificado == 1) {
          asignar_lugar_en_cache(index,pid,nro_pagina,valor_a_escribir);
          parametros_cache->puntero_reemplazo = (index + 1) % cantidad;
          return;
          }

          else {
          cache[index].bit_uso=0;
          }
        }
        vueltas++;
      }

      int index = parametros_cache->puntero_reemplazo;
      asignar_lugar_en_cache(index, pid, nro_pagina, valor_a_escribir);
      parametros_cache->puntero_reemplazo = (index + 1) % cantidad;
    return;
  }
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
}

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
  int ultimo_agregado = 0;
  int reemplazo;
  
  //busco entrada libre
  for(int i= 0; i < parametros_tlb -> cantidad_entradas; i++){
    if(tlb[i].pid == -1){
      ultimo_agregado = i;
      asignar_lugar_en_TLB(i,pid,marco,pagina,ultimo_agregado);
    return;
    }
  }

  switch (parametros_tlb->algoritmo_reemplazo){
    case LRU:
    for (int i=0; i < parametros_tlb -> cantidad_entradas; i++){
      if(tlb[i].tiempo_transcurrido > index_tiempo) {
        reemplazo = i;
      }
    }
    ultimo_agregado +=1;
    asignar_lugar_en_TLB(reemplazo,pid,marco,pagina,ultimo_agregado);
    break;

    case FIFO:
    for (int i=0;i <= parametros_tlb->cantidad_entradas;i++) {
      if (tlb[i].nro_orden < index_orden) {
        reemplazo = i;
      }
    }
    ultimo_agregado +=1;
    asignar_lugar_en_TLB(reemplazo,pid,marco,pagina,ultimo_agregado);
    break;
  }
} 

//Auxiliar para hacer los reemplazos y asignaciones en aché y TLB
void asignar_lugar_en_cache(int ubicacion, uint32_t pid, uint32_t pagina,char* valor){
  cache[ubicacion].pid=pid;
  cache[ubicacion].pagina=pagina;
  cache[ubicacion].contenido=valor;
  cache[ubicacion].bit_uso=1;
  cache[ubicacion].bit_modificado=1;
}

void asignar_lugar_en_TLB(int ubicacion, uint32_t pid, uint32_t marco, uint32_t pagina, int ultimo_agregado) {
  tlb[ubicacion].pid = pid;
  tlb[ubicacion].pagina = pagina;
  tlb[ubicacion].marco = marco;
  tlb[ubicacion].nro_orden = ultimo_agregado;
  tlb[ubicacion].tiempo_transcurrido = 0;
}

char* pedir_valor_a_memoria(uint32_t direccion_fisica, char* tamaño){
  uint32_t tamaño_a_leer = (uint32_t) atoi(tamaño);
  t_paquete* paquete = crear_paquete(LECTURA);

  agregar_a_paquete(paquete, &direccion_fisica, sizeof(uint32_t));
  agregar_a_paquete(paquete, &tamaño_a_leer, sizeof(uint32_t));

  enviar_paquete(paquete, conexion_memoria);

  char* valor;

  recv(conexion_memoria, &valor, sizeof(char*), MSG_WAITALL);

  return valor;
}

void escribir_valor_en_memoria(uint32_t direccion_fisica, char* valor){
  uint32_t longitud_valor = (uint32_t) sizeof(valor);
  
  t_paquete* paquete = crear_paquete(ESCRITURA);

  agregar_a_paquete(paquete, &direccion_fisica, sizeof(uint32_t));
  agregar_a_paquete(paquete, &valor, sizeof(char*));
  agregar_a_paquete(paquete,&longitud_valor,sizeof(uint32_t));

  enviar_paquete(paquete, conexion_memoria);
};
