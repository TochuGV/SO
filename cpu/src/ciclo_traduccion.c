#include "ciclo_traduccion.h"

//Caché de páginas
//verificar si tengo el valor que necesito leer
char* consultar_contenido_cache (t_cpu* cpu, uint32_t pid, uint32_t nro_pagina) {
  usleep((cpu->retardo_cache)*1000);
  log_debug(logger,"Consultando Caché en busca del contenido a leer");
  for (int i=0;i<cpu->parametros_cache->cantidad_entradas ;i++) {
    //log_debug(logger,"PID: <%d> - PAGINA: <%d> - CONTENIDO: <%s>",cpu->cache[i].pid,cpu->cache[i].pagina,cpu->cache[i].contenido);
    if (cpu->cache[i].pid==pid && cpu->cache[i].pagina==nro_pagina) {
        //Log 8. Página encontrada en Caché
        log_info(logger, "PID: <%d> - CACHE HIT - Pagina: <%d>", pid, nro_pagina);
        
        return cpu->cache[i].contenido;
    }
  }
  //Log 7. Página faltante en Caché
  log_info(logger, "PID: <%d> - CACHE MISS - Pagina: <%d>", pid, nro_pagina);
  return NULL;
}

//Verificar si tengo la página que estoy buscando escribir
int consultar_pagina_cache (t_cpu* cpu, uint32_t pid, uint32_t nro_pagina) {
  usleep((cpu->retardo_cache)*1000);
  log_debug(logger,"Consultando Caché en busca de la página a escribir");
  for (int i=0;i<cpu->parametros_cache->cantidad_entradas ;i++) {
    //log_debug(logger,"PID: <%d> - PAGINA: <%d> - CONTENIDO: <%s>",cpu->cache[i].pid,cpu->cache[i].pagina,cpu->cache[i].contenido);
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
void actualizar_cache(t_cpu* cpu, uint32_t pid,uint32_t nro_pagina,char* valor_a_escribir, bool es_escritura, uint32_t desplazamiento) {

  int cantidad=cpu->parametros_cache->cantidad_entradas;

  //busco entrada libre
  for(int i= 0; i < cpu->parametros_cache->cantidad_entradas; i++){
    if(cpu-> cache[i].pid == -1){
      asignar_lugar_en_cache(cpu,i,pid,nro_pagina,valor_a_escribir,es_escritura,desplazamiento);
      cpu->parametros_cache->puntero_reemplazo= (i+1) % cantidad;
    return;
    }
  }
  
  switch (cpu->parametros_cache->algoritmo_reemplazo){
    case CLOCK:
      while(true){
        if(cpu->cache[cpu->parametros_cache->puntero_reemplazo].bit_uso == 0) {
          asignar_lugar_en_cache(cpu,cpu->parametros_cache->puntero_reemplazo,pid,nro_pagina,valor_a_escribir,es_escritura,desplazamiento);
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
      int inicio = cpu->parametros_cache->puntero_reemplazo;
      while (vueltas < 4) {
        log_debug(logger,"Vuelta %d",vueltas);
        for (int i = 0; i < cantidad; i++) {
          int index = (inicio +i) % cantidad;
          log_debug(logger,"PID: <%d> - PAGINA: <%d> - USO: <%d> - MODIFICADO: <%d>",cpu->cache[i].pid,cpu->cache[i].pagina,cpu->cache[i].bit_uso,cpu->cache[i].bit_modificado);
          if(vueltas==0 && cpu->cache[index].bit_uso == 0 && cpu->cache[index].bit_modificado == 0) {
          log_debug(logger,"Voy a reemplazar PAGINA: <%d> - USO: <%d> - MODIFICADO: <%d>",cpu->cache[i].pagina,cpu->cache[i].bit_uso,cpu->cache[i].bit_modificado);
          log_debug(logger,"i:%d - puntero:%d ",i,cpu->parametros_cache->puntero_reemplazo);
          asignar_lugar_en_cache(cpu,index,pid,nro_pagina,valor_a_escribir,es_escritura,desplazamiento);
          cpu->parametros_cache->puntero_reemplazo = (index + 1) % cantidad;
          return;
          }

          if(vueltas==1 && cpu->cache[index].bit_uso == 0 && cpu->cache[index].bit_modificado == 1) {
          log_debug(logger,"Voy a reemplazar PAGINA: <%d> - USO: <%d> - MODIFICADO: <%d>",cpu->cache[i].pagina,cpu->cache[i].bit_uso,cpu->cache[i].bit_modificado);
          log_debug(logger,"i:%d - puntero:%d ",i,cpu->parametros_cache->puntero_reemplazo);
          asignar_lugar_en_cache(cpu,index,pid,nro_pagina,valor_a_escribir,es_escritura,desplazamiento);
          cpu->parametros_cache->puntero_reemplazo = (index+ 1) % cantidad;
          return;
          }

          if(vueltas==1){
          cpu->cache[index].bit_uso=0;
          }

          if(vueltas==2 && cpu->cache[index].bit_uso == 0 && cpu->cache[index].bit_modificado == 0) {
          log_debug(logger,"Voy a reemplazar PAGINA: <%d> - USO: <%d> - MODIFICADO: <%d>",cpu->cache[i].pagina,cpu->cache[i].bit_uso,cpu->cache[i].bit_modificado);
          log_debug(logger,"i:%d - puntero:%d ",i,cpu->parametros_cache->puntero_reemplazo);
          asignar_lugar_en_cache(cpu,index,pid,nro_pagina,valor_a_escribir,es_escritura,desplazamiento);
          cpu->parametros_cache->puntero_reemplazo = (index + 1) % cantidad;
          return;
          }

          if(vueltas==3 && cpu->cache[index].bit_uso == 0 && cpu->cache[index].bit_modificado == 1) {
          log_debug(logger,"Voy a reemplazar PAGINA: <%d> - USO: <%d> - MODIFICADO: <%d>",cpu->cache[i].pagina,cpu->cache[i].bit_uso,cpu->cache[i].bit_modificado);
          log_debug(logger,"i:%d - puntero:%d ",i,cpu->parametros_cache->puntero_reemplazo);
          asignar_lugar_en_cache(cpu,index,pid,nro_pagina,valor_a_escribir,es_escritura,desplazamiento);
          cpu->parametros_cache->puntero_reemplazo = (index+ 1) % cantidad;
          return;
          }
        }
        vueltas++;
      }
      log_warning(logger,"estoy afuera del While");
  }
}

//Finaliza un proceso en caché actualizando memoria, si finalizó el proceso, libera la entrada
void finalizar_proceso_en_cache(t_cpu* cpu, uint32_t pid) {
  //log_debug(logger,"Finalizando proceso, actualización de Memoria iniciada");
    for (int i = 0; i < cpu->parametros_cache->cantidad_entradas; i++) {
        if (cpu->cache[i].pid == pid) {
            if (cpu->cache[i].bit_modificado == 1) {
                uint32_t direccion_fisica = traducir_direccion(cpu,pid, cpu->cache[i].pagina, cpu->cache[i].desplazamiento);
                escribir_valor_en_memoria(cpu,pid, direccion_fisica, cpu->cache[i].contenido, ACTUALIZAR_PAGINA);
            }
            cpu->cache[i].pid = -1;
            cpu->cache[i].pagina = -1;
            cpu->cache[i].contenido = NULL;
            cpu->cache[i].bit_uso = 0;
            cpu->cache[i].bit_modificado = 0;
            cpu->cache[i].desplazamiento = 0;
        }
    }
  //log_debug(logger,"Actualización de Memoria finalizada");
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
      log_debug(logger, "No se pudo obtener el marco para la página <%d> del PID <%d>", nro_pagina, pid);
      return -1;
    };
    
    if (cpu->parametros_tlb->cantidad_entradas>0) {
        actualizar_TLB(cpu,pid, nro_pagina, marco);
     }
  }
  //Log 5. Obtener Marco
  log_info(logger, "PID: <%d> - OBTENER MARCO - Página: <%d> - Marco: <%d>", pid, nro_pagina, marco);
  return marco * tamanio_pagina + desplazamiento;
}

//Consultar TLB si falló en Caché
uint32_t consultar_TLB (t_cpu* cpu,uint32_t pid, uint32_t nro_pagina) {
  log_debug(logger,"Iniciando consulta a TLB");
  for (int i=0;i<cpu->parametros_tlb->cantidad_entradas ;i++) {
    log_debug(logger, "PID: <%d> - PAGINA: <%d> - MARCO: <%d> - TIEMPO: <%d>",cpu->tlb[i].pid,cpu->tlb[i].pagina,cpu->tlb[i].marco,cpu->tlb[i].tiempo_transcurrido);
    cpu->tlb[i].tiempo_transcurrido++;
    if (cpu->tlb[i].pid==pid && cpu->tlb[i].pagina==nro_pagina) {
        cpu->tlb[i].tiempo_transcurrido = 0;
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
  int index_tiempo = 0;
  int reemplazo;
  
  //busco entrada libre
  for(int i= 0; i < cpu->parametros_tlb -> cantidad_entradas; i++){
    if(cpu->tlb[i].pid == -1){
      cpu->orden_actual_tlb = i;
      asignar_lugar_en_TLB(cpu,i,pid,marco,pagina,cpu->orden_actual_tlb);
    return;
    }
  }

  switch (cpu->parametros_tlb->algoritmo_reemplazo){
    case LRU:
    for (int i=0; i < cpu->parametros_tlb->cantidad_entradas; i++){
      if(cpu->tlb[i].tiempo_transcurrido > index_tiempo) {
        reemplazo = i;
        index_tiempo = cpu->tlb[i].tiempo_transcurrido;
      }
    }
    cpu->orden_actual_tlb ++;
    asignar_lugar_en_TLB(cpu,reemplazo,pid,marco,pagina,cpu->orden_actual_tlb);
    break;

    case FIFO:
    for (int i=0;i < cpu->parametros_tlb->cantidad_entradas;i++) {
      if (cpu->tlb[i].nro_orden < index_orden) {
        reemplazo = i;
        index_orden = cpu->tlb[i].nro_orden;
      }
    }
    cpu->orden_actual_tlb ++;
    asignar_lugar_en_TLB(cpu,reemplazo,pid,marco,pagina,cpu->orden_actual_tlb);
    break;
  }
} 

//Auxiliar para hacer los reemplazos y asignaciones en caché (y actualizar memoria si corresponde)
void asignar_lugar_en_cache(t_cpu* cpu, int ubicacion, uint32_t pid, uint32_t pagina,char* valor, bool es_escritura, uint32_t desplazamiento){
  if (cpu->cache[ubicacion].bit_modificado==1) {
    log_debug(logger,"La página estaba modificada, iniciando actualización en memoria");

    uint32_t direccion_fisica=traducir_direccion(cpu,pid,cpu->cache[ubicacion].pagina, cpu->cache[ubicacion].desplazamiento);

    uint32_t marco = (direccion_fisica - cpu->cache[ubicacion].desplazamiento) / tamanio_pagina;

    log_info(logger,"PID: <%d> - MEMORY UPDATE - Pagina: <%d> - Frame: <%d>",cpu->cache[ubicacion].pid,cpu->cache[ubicacion].pagina, marco);
    
    escribir_valor_en_memoria(cpu,cpu->cache[ubicacion].pid, direccion_fisica,cpu->cache[ubicacion].contenido, ACTUALIZAR_PAGINA);
    //free(cpu->cache[ubicacion].contenido);
  }

  //Log 9. Página ingresada en Cache
  log_info(logger,"PID: <%d> - CACHE ADD - Pagina: <%d>",pid,pagina);
  log_debug(logger,"Estoy reemplazando la pagina: %d", cpu->cache[ubicacion].pagina);
  cpu->cache[ubicacion].pid=pid;
  cpu->cache[ubicacion].pagina=pagina;
  cpu->cache[ubicacion].contenido=valor;
  cpu->cache[ubicacion].bit_uso=1;
  cpu->cache[ubicacion].bit_modificado=0;
  if(es_escritura) {
    cpu->cache[ubicacion].bit_modificado=1;
  }
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
    list_destroy_and_destroy_elements(valores,free);
  }

  return valor;
}

void escribir_valor_en_memoria(t_cpu* cpu, uint32_t pid, uint32_t direccion_fisica, char* valor, int cod_op){
  uint32_t longitud_valor = strlen(valor);
  
  t_paquete* paquete = crear_paquete(cod_op);

  agregar_a_paquete(paquete, &pid, sizeof(uint32_t));
  agregar_a_paquete(paquete, &direccion_fisica, sizeof(uint32_t));
  agregar_a_paquete(paquete, &longitud_valor, sizeof(uint32_t));
  agregar_a_paquete(paquete, valor, longitud_valor);

  free(valor);

  enviar_paquete(paquete, cpu->conexion_memoria);
};
