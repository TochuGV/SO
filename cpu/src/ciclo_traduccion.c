#include "ciclo_traduccion.h"

//Caché de páginas
//verificar si tengo el valor que necesito leer
char* consultar_contenido_cache (t_cpu* cpu, uint32_t pid, uint32_t nro_pagina) {
  for (int i=0;i<cpu->parametros_cache->cantidad_entradas ;i++) {
    if (cpu->cache[i].pid==pid && cpu->cache[i].pagina==nro_pagina) {
        //Log 8. Página encontrada en Caché
        log_info(logger, "PID: <%d> - Cache Hit - Pagina: <%d>", pid, nro_pagina);
        
        return cpu->cache[i].contenido;
    }
  }
  //Log 7. Página faltante en Caché
  log_info(logger, "PID: <%d> - Cache Miss - Pagina: <%d>", pid, nro_pagina);
  return NULL;
}

//Verificar si tengo la página que estoy buscando escribir
int consultar_pagina_cache (t_cpu* cpu, uint32_t pid, uint32_t nro_pagina) {
  for (int i=0;i<cpu->parametros_cache->cantidad_entradas ;i++) {
    if (cpu->cache[i].pid == pid && cpu->cache[i].pagina==nro_pagina) {
        //Log 8. Página encontrada en Caché
        log_info(logger, "PID: <%d> - CACHE HIT - Pagina: <%d>", pid, nro_pagina);
        
        return i;  
    }
  }
  //Log 7. Página faltante en Caché
  log_info(logger, "PID: <%d> - CACHE MISS - Pagina: <%d>", pid, nro_pagina);
  return -1;
}

//Actualiza la caché con el nuevo valor según el algoritmo
void actualizar_cache(t_cpu* cpu, uint32_t pid,uint32_t nro_pagina,char* valor_a_escribir, bool es_escritura) {
  int cantidad=cpu->parametros_cache->cantidad_entradas;

  //busco entrada libre
  for(int i= 0; i < cpu->parametros_tlb->cantidad_entradas; i++){
    if(cpu-> cache[i].pid == -1){
      asignar_lugar_en_cache(cpu,i,pid,nro_pagina,valor_a_escribir,es_escritura);
      cpu->parametros_cache->puntero_reemplazo= (i+1) % cantidad;
    return;
    }
  }
  
  switch (cpu->parametros_cache->algoritmo_reemplazo){
    case CLOCK:
      while(true){
        if(cpu->cache[cpu->parametros_cache->puntero_reemplazo].bit_uso == 0) {
          asignar_lugar_en_cache(cpu,cpu->parametros_cache->puntero_reemplazo,pid,nro_pagina,valor_a_escribir,es_escritura);
          cpu->parametros_cache->puntero_reemplazo = (cpu->parametros_cache->puntero_reemplazo + 1) % cantidad;
          return;
        }
        else {
          cpu->cache[cpu->parametros_cache->puntero_reemplazo].bit_uso=0;
          cpu->parametros_cache->puntero_reemplazo = (cpu->parametros_cache->puntero_reemplazo + 1) % cantidad;
        }
      }
    break;

    case CLOCK_M:
      int vueltas = 0;
      while (vueltas < 2) {
        for (int i = 0; i < cantidad; i++) {
          int index = (cpu->parametros_cache->puntero_reemplazo + i) % cantidad;

          if(vueltas==0 && cpu->cache[index].bit_uso == 0 && cpu->cache[index].bit_modificado == 0) {
          asignar_lugar_en_cache(cpu,index,pid,nro_pagina,valor_a_escribir,es_escritura);
          cpu->parametros_cache->puntero_reemplazo = (index + 1) % cantidad;
          return;
          }

          if(vueltas==1 && cpu->cache[index].bit_uso == 0 && cpu->cache[index].bit_modificado == 1) {
          asignar_lugar_en_cache(cpu,index,pid,nro_pagina,valor_a_escribir,es_escritura);
          cpu->parametros_cache->puntero_reemplazo = (index + 1) % cantidad;
          return;
          }

          else {
          cpu->cache[index].bit_uso=0;
          }
        }
        vueltas++;
      }

      int index = cpu->parametros_cache->puntero_reemplazo;
      asignar_lugar_en_cache(cpu,index, pid, nro_pagina, valor_a_escribir,es_escritura);
      cpu->parametros_cache->puntero_reemplazo = (index + 1) % cantidad;
    return;
  }
}

//Finaliza un proceso en caché actualizando memoria, si finalizó el proceso, libera la entrada
void finalizar_proceso_en_cache(t_cpu* cpu, uint32_t pid,t_estado_ejecucion estado) {
    for (int i = 0; i < cpu->parametros_cache->cantidad_entradas; i++) {
        if (cpu->cache[i].pid == pid) {
            if (cpu->cache[i].bit_modificado == 1) {
                uint32_t direccion_fisica = traducir_direccion(cpu,pid, cpu->cache[i].pagina, 0);
                escribir_valor_en_memoria(cpu,pid, direccion_fisica, cpu->cache[i].contenido);
            }
            if (estado==EJECUCION_FINALIZADA){
            cpu->cache[i].pid = -1;
            cpu->cache[i].pagina = -1;
            cpu->cache[i].contenido = NULL;
            cpu->cache[i].bit_uso = 0;
            cpu->cache[i].bit_modificado = 0;
            }
        }
    }
}

//MMU
uint32_t traducir_direccion(t_cpu* cpu, uint32_t pid, uint32_t nro_pagina,uint32_t desplazamiento) {

  uint32_t marco = -1;

  if (cpu->parametros_tlb->cantidad_entradas>0) {
    marco = consultar_TLB(cpu, pid, nro_pagina);
  }
  
  if(marco ==-1){
    marco = consultar_memoria(cpu, pid, nro_pagina);

    if(marco == -1){
      log_error(logger, "No se pudo obtener el marco para la página <%d> del PID <%d>", nro_pagina, pid);
      return -1;
    };
    
    actualizar_TLB(cpu,pid, nro_pagina, marco);
    }
  //Log 5. Obtener Marco
  log_info(logger, "PID: <%d> - OBTENER MARCO - Página: <%d> - Marco: <%d>", pid, nro_pagina, marco);
  return marco * tamanio_pagina + desplazamiento;
}

//Consultar TLB si falló en Caché
uint32_t consultar_TLB (t_cpu* cpu,uint32_t pid, uint32_t nro_pagina) {
  for (int i=0;i<cpu->parametros_tlb->cantidad_entradas ;i++) {
    cpu->tlb[i].tiempo_transcurrido++;
    if (cpu->tlb[i].pid==pid && cpu->tlb[i].pagina==nro_pagina) {
        cpu->tlb[i].nro_orden = i;
        //Log 6. TLB Hit
        log_info(logger, "PID: <%d> - TLB HIT - Pagina: <%d>", pid, nro_pagina);
        return cpu->tlb[i].marco;
    }
  } 
  //Log 7. TLB Miss
  log_info(logger, "PID: <%d> - TLB MISS - Pagina: <%d>", pid, nro_pagina);
  return -1;
}

//Consultar memoria si falló en TLB
uint32_t consultar_memoria(t_cpu* cpu, uint32_t pid, uint32_t nro_pagina) {

  uint32_t marco;
  t_paquete* paquete = crear_paquete(SOLICITUD_MARCO);
  agregar_a_paquete(paquete, &pid, sizeof(uint32_t)); 

  for (int nivel=1; nivel <= cant_niveles; nivel++) {
    uint32_t entrada_nivel = (uint32_t) floor(nro_pagina / pow(cant_entradas_tabla,cant_niveles - nivel)) % cant_entradas_tabla;
    agregar_a_paquete(paquete, &entrada_nivel, sizeof(uint32_t)); 
  }

  enviar_paquete(paquete, cpu->conexion_memoria);
  recv(cpu->conexion_memoria, &marco, sizeof(uint32_t), MSG_WAITALL);
  return marco;
}


//Insertar un nuevo marco en el TLB
void actualizar_TLB (t_cpu* cpu, uint32_t pid, uint32_t pagina, uint32_t marco) {
  int index_orden = 10000;
  int index_tiempo = -1;
  int ultimo_agregado = 0;
  int reemplazo;
  
  //busco entrada libre
  for(int i= 0; i < cpu->parametros_tlb -> cantidad_entradas; i++){
    if(cpu->tlb[i].pid == -1){
      ultimo_agregado = i;
      asignar_lugar_en_TLB(cpu,i,pid,marco,pagina,ultimo_agregado);
    return;
    }
  }

  switch (cpu->parametros_tlb->algoritmo_reemplazo){
    case LRU:
    for (int i=0; i < cpu->parametros_tlb -> cantidad_entradas; i++){
      if(cpu->tlb[i].tiempo_transcurrido > index_tiempo) {
        reemplazo = i;
      }
    }
    ultimo_agregado +=1;
    asignar_lugar_en_TLB(cpu,reemplazo,pid,marco,pagina,ultimo_agregado);
    break;

    case FIFO:
    for (int i=0;i <= cpu->parametros_tlb->cantidad_entradas;i++) {
      if (cpu->tlb[i].nro_orden < index_orden) {
        reemplazo = i;
      }
    }
    ultimo_agregado +=1;
    asignar_lugar_en_TLB(cpu,reemplazo,pid,marco,pagina,ultimo_agregado);
    break;
  }
} 

//Auxiliar para hacer los reemplazos y asignaciones en caché (y actualizar memoria si corresponde)
void asignar_lugar_en_cache(t_cpu* cpu, int ubicacion, uint32_t pid, uint32_t pagina,char* valor, bool es_escritura){
  if (cpu->cache[ubicacion].bit_modificado==1) {
    uint32_t direccion_fisica=traducir_direccion(cpu,pid,cpu->cache[ubicacion].pagina,0);
    //Log 10. Pagina Actualizada de Cache a Memoria
    //Como actualizo una página completa, no conozco el frame
    log_info(logger,"PID: <%d> - MEMORY UPDATE - Pagina: <%d>",pid,pagina);
    
    escribir_valor_en_memoria(cpu,pid, direccion_fisica,cpu->cache[ubicacion].contenido);
  }

  //Log 9. Página ingresada en Cache
  log_info(logger,"PID: <%d> - CACHE ADD - Pagina: <%d>",pid,pagina);

  cpu->cache[ubicacion].pid=pid;
  cpu->cache[ubicacion].pagina=pagina;
  cpu->cache[ubicacion].contenido=valor;
  cpu->cache[ubicacion].bit_uso=1;
  cpu->cache[ubicacion].bit_modificado = es_escritura? 1 : 0;
}

//Auxiliar para hacer los reemplazos y asignaciones en TLB
void asignar_lugar_en_TLB(t_cpu* cpu,int ubicacion, uint32_t pid, uint32_t marco, uint32_t pagina, int ultimo_agregado) {
  cpu->tlb[ubicacion].pid = pid;
  cpu->tlb[ubicacion].pagina = pagina;
  cpu->tlb[ubicacion].marco = marco;
  cpu->tlb[ubicacion].nro_orden = ultimo_agregado;
  cpu->tlb[ubicacion].tiempo_transcurrido = 0;
}

char* pedir_valor_a_memoria(t_cpu* cpu, uint32_t pid, uint32_t direccion_fisica, char* tamanio){
  uint32_t tamanio_a_leer = atoi(tamanio);
  t_paquete* paquete = crear_paquete(LECTURA);

  agregar_a_paquete(paquete, &pid, sizeof(uint32_t));
  agregar_a_paquete(paquete, &direccion_fisica, sizeof(uint32_t));
  agregar_a_paquete(paquete, &tamanio_a_leer, sizeof(uint32_t));

  enviar_paquete(paquete, cpu->conexion_memoria);

  int cod_op = recibir_operacion(cpu->conexion_memoria);


  char* valor;

  if (cod_op == LECTURA) {
    t_list* valores = recibir_paquete(cpu->conexion_memoria);

    uint32_t longitud_valor = *(uint32_t*) list_get(valores, 0);

    valor = malloc(longitud_valor);
    memcpy(valor, list_get(valores, 1), longitud_valor);
    log_debug(logger, "El valor leido: <%s>, fue recibido", valor);
  }

  return valor;
}

void escribir_valor_en_memoria(t_cpu* cpu, uint32_t pid, uint32_t direccion_fisica, char* valor){
  uint32_t longitud_valor = strlen(valor) + 1;
  
  t_paquete* paquete = crear_paquete(ESCRITURA);

  agregar_a_paquete(paquete, &pid, sizeof(uint32_t));
  agregar_a_paquete(paquete, &direccion_fisica, sizeof(uint32_t));
  agregar_a_paquete(paquete, &longitud_valor, sizeof(uint32_t));
  agregar_a_paquete(paquete, valor, longitud_valor);

  enviar_paquete(paquete, cpu->conexion_memoria);
};
