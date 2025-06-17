#include "ciclo_traduccion.h"

estructura_tlb* tlb;
tlb_t* parametros_tlb;
uint32_t tamanio_pagina;
uint32_t cant_entradas_tabla;
uint32_t cant_niveles;

//MMU
uint32_t traducir_direccion(uint32_t pid, uint32_t direccion_logica, uint32_t parametro) {
  
  //Calcular numero de pagina y desplazamiento 
  uint32_t nro_pagina = floor(direccion_logica / tamanio_pagina);
  uint32_t desplazamiento = direccion_logica % tamanio_pagina;
  //uint32_t tabla_actual;

  uint32_t marco = consultar_cache(pid, nro_pagina);

  if(marco==-1){
    marco = consultar_TLB(pid, nro_pagina);

    if(marco ==-1){
      marco = consultar_memoria(pid, nro_pagina);

      if(marco == -1){
        log_error(logger, "No se pudo obtener el marco para la página %d del PID %d", nro_pagina, pid);
        return -1;
      };
      
      //actualizar_TLB(nro_pagina, marco);
    }
  }
  //Log 5. Obtener Marco
  log_info(logger, "PID: %d - OBTENER MARCO - Página: %d - Marco: %d", pid, nro_pagina, marco);
  return marco * tamanio_pagina + desplazamiento;
};

//Consultar Caché
uint32_t consultar_cache (uint32_t pid, uint32_t nro_pagina) {
  return -1; //Por ahora manda siempre a TLB
}; 

//Consultar TLB si falló en Caché
uint32_t consultar_TLB (uint32_t pid, uint32_t nro_pagina) {
  for (int i=0;i<parametros_tlb->cantidad_entradas ;i++) {
    if (tlb[i].pid==pid && tlb[i].pagina==nro_pagina) {
        tlb [i]. tiempo_transcurrido = 0;
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

/*
//Insertar un nuevo marco en el TLB
void actualizar_TLB (uint32_t pid, int pagina, int marco) {
  
  switch (parametros_tlb->algoritmo_reemplazo){
    case "LRU":
    //Se fija cual es el que no usa hace más tiempo y lo reemplaza
    break;

    case "FIFO":
    //Se fija cual es el que entró hace más tiempo y lo reemplaza
    break;
  }
}
*/

void pedir_valor_a_memoria(uint32_t direccion_fisica, uint32_t* valor){
  t_paquete* paquete = crear_paquete(OP_READ);
  agregar_a_paquete(paquete, &direccion_fisica, sizeof(uint32_t));
  enviar_paquete(paquete, conexion_memoria);
  recv(conexion_memoria, valor, sizeof(uint32_t), MSG_WAITALL);
}

void escribir_valor_en_memoria(uint32_t direccion_fisica, uint32_t valor){
  t_paquete* paquete = crear_paquete(OP_WRITE);
  agregar_a_paquete(paquete, &direccion_fisica, sizeof(uint32_t));
  agregar_a_paquete(paquete, &valor, sizeof(uint32_t));
  enviar_paquete(paquete, conexion_memoria);
}

