#include "ciclo_instruccion.h"

//Realizar todo el ciclo de cada instrucción
void* ciclo_de_instruccion(t_cpu* cpu, t_pcb* pcb, int conexion_kernel_dispatch, int conexion_kernel_interrupt, int conexion_memoria){
  t_list* lista_instruccion;
  t_instruccion instruccion;
  t_estado_ejecucion estado = EJECUCION_CONTINUA;

  while(estado == EJECUCION_CONTINUA || estado == EJECUCION_CONTINUA_INIT_PROC){
    //Log 1. Fetch Instrucción
    log_info (logger, "## PID: <%d> - FETCH - Program Counter: <%d>", pcb->pid, pcb->pc);
    
    lista_instruccion = recibir_instruccion(pcb, conexion_memoria);
    if (list_size(lista_instruccion) == 0) {
      log_warning(logger, "No se recibió ninguna instrucción desde Memoria");
    };
    
    instruccion.tipo = *(int*) list_get(lista_instruccion, 0);

    uint32_t longitud_parametro1 = *(uint32_t*)list_get(lista_instruccion, 1); 
    instruccion.parametro1 = malloc(longitud_parametro1);
    memcpy(instruccion.parametro1, list_get(lista_instruccion, 2), longitud_parametro1); 

    uint32_t longitud_parametro2 = *(uint32_t*)list_get(lista_instruccion, 3); 
    instruccion.parametro2 = malloc(longitud_parametro2);
    memcpy(instruccion.parametro2, list_get(lista_instruccion, 4), longitud_parametro2); 

    list_destroy_and_destroy_elements(lista_instruccion, free);
    estado = trabajar_instruccion(cpu, instruccion, pcb);

    if(estado == EJECUCION_CONTINUA_INIT_PROC){
      actualizar_kernel(instruccion, estado, pcb, conexion_kernel_dispatch);
    };

    if (estado == EJECUCION_CONTINUA_INIT_PROC || estado == EJECUCION_CONTINUA) {
      if (chequear_interrupcion(conexion_kernel_interrupt, pcb->pid)) {
        //log_warning(logger, "PID %d interrumpido", pcb->pid);
        estado = EJECUCION_DESALOJADO;
      };
    }
  }
  actualizar_kernel(instruccion, estado, pcb, conexion_kernel_dispatch);
  //finalizar_proceso_en_cache(pcb->pid,estado);
  free(pcb);
  return NULL;
}

//Recibir información del PCB desde Kernel
t_pcb* recibir_pcb(int conexion_kernel_dispatch) {
  int cod_op = recibir_operacion(conexion_kernel_dispatch);
  if(cod_op != PCB) {
    return NULL;
  };
  
  t_list* campos = recibir_paquete(conexion_kernel_dispatch);
  if(list_size(campos) == 0) return NULL;
  void* buffer = list_get(campos, 0);
  t_pcb* pcb = deserializar_pcb(buffer);
  list_destroy_and_destroy_elements(campos, free);
  return pcb;
};

t_pcb* deserializar_pcb(void* buffer) {
  t_pcb* pcb = malloc(sizeof(t_pcb));
  int offset = 0;

  memcpy(&(pcb->pid), buffer + offset, sizeof(uint32_t));
  offset += sizeof(uint32_t);


  memcpy(&(pcb->pc), buffer + offset, sizeof(uint32_t));
  offset += sizeof(uint32_t);
  
  return pcb;
}

//Solicitar instrucción a Memoria
t_list* recibir_instruccion(t_pcb* pcb, int conexion_memoria) {

  t_paquete* paquete = crear_paquete(SOLICITUD_INSTRUCCION);
  agregar_a_paquete(paquete, &pcb->pid, sizeof(uint32_t));
  agregar_a_paquete(paquete, &pcb->pc, sizeof(uint32_t));
  enviar_paquete(paquete, conexion_memoria);

  t_list* instruccion;
  int cod_op = recibir_operacion(conexion_memoria);
  if (cod_op == INSTRUCCION)
  {
    instruccion = recibir_paquete(conexion_memoria);
  }

  return instruccion;
}

//Decode y Execute Instrucción
t_estado_ejecucion trabajar_instruccion(t_cpu* cpu, t_instruccion instruccion, t_pcb* pcb){
  switch (instruccion.tipo) {
  //El log_info en cada Case corresponde a Log 3. Instrucción Ejecutada
    case NOOP:
      log_info (logger, "## PID: <%d> - Ejecutando: <NOOP>", pcb->pid);
      sleep(1); //Uso una duración de 1 segundo como default para NOOP
      pcb->pc++;
      free(instruccion.parametro1);
      free(instruccion.parametro2);
      return EJECUCION_CONTINUA;
      break;
    
    case READ:
      log_info(logger, "## PID: <%d> - Ejecutando: <READ> - Direccion logica: %s - tamaño: %s", pcb->pid, instruccion.parametro1, instruccion.parametro2);
      ejecutar_read(cpu,pcb->pid,instruccion.parametro1, instruccion.parametro2);
      pcb->pc++;
      return EJECUCION_CONTINUA;
      break;
    
    case WRITE: 
      log_info(logger, "## PID: <%d> - Ejecutando: <WRITE> - Direccion logica: <%s> - Valor: <%s> ", pcb->pid, instruccion.parametro1, instruccion.parametro2);
      ejecutar_write(cpu,pcb->pid,instruccion.parametro1, instruccion.parametro2);
      pcb->pc++;
      return EJECUCION_CONTINUA;
      break;
    
    case GOTO: 
      log_info(logger, "## PID: <%d> - Ejecutando: <GOTO> - Nuevo PC: <%s>", pcb->pid, instruccion.parametro1);
      pcb->pc = atoi(instruccion.parametro1);
      free(instruccion.parametro1);
      free(instruccion.parametro2);
      return EJECUCION_CONTINUA;
      break;
    
    case IO: 
      log_info(logger, "## PID: <%d> - Ejecutando: <IO> - Dispositivo: <%s> - Tiempo: <%s>", pcb->pid, instruccion.parametro1, instruccion.parametro2);
      pcb->pc++;
      return EJECUCION_BLOQUEADA_IO;
      break;
    
    case INIT_PROC:
      log_info(logger, "## PID: <%d> - Ejecutando: <INIT_PROC> - Archivo: <%s> - Tamaño: <%s>", pcb->pid, instruccion.parametro1, instruccion.parametro2);
      pcb->pc++;
      return EJECUCION_CONTINUA_INIT_PROC;
      break;

    case DUMP_MEMORY:
      log_info(logger,"## PID: <%d> - Ejecutando: <DUMP_MEMORY>", pcb->pid);
      pcb->pc++;
      return EJECUCION_BLOQUEADA_DUMP;
      break;

    case EXIT:
      log_info(logger, "## PID: <%d> - Ejecutando: <EXIT>", pcb->pid);
      pcb->pc++;
      finalizar_proceso_en_cache(cpu,pcb->pid);
      return EJECUCION_FINALIZADA;
      break;
    
    default: 
      log_warning(logger, "## PID: <%d> - Instrucción desconocida: <%d>", pcb->pid, instruccion.tipo);
      break;
  };
  return EJECUCION_FINALIZADA;
};

//Ejecutar Write y Read
void ejecutar_read (t_cpu* cpu, uint32_t pid, char* direccion_logica, char* parametro2){
  int direccion = atoi(direccion_logica);
  //Calcular numero de pagina y desplazamiento 
  uint32_t nro_pagina = floor(direccion / tamanio_pagina);
  uint32_t desplazamiento = direccion % tamanio_pagina;
  uint32_t direccion_fisica;
  char* valor_a_leer = NULL;
  bool es_escritura=false;

  if (cpu->parametros_cache->cantidad_entradas > 0) {
    valor_a_leer = consultar_contenido_cache(cpu,pid,nro_pagina);
  }
  
  //direccion_fisica = traducir_direccion(cpu,pid,nro_pagina,desplazamiento);

  if (valor_a_leer != NULL) {
    //Al haber habido Caché Hit, no contamos con la dirección física
    log_info(logger, "PID: <%d> - Acción: <LEER> - Valor: <%s>", pid, valor_a_leer);
    return; 
  }  


  if (cpu->parametros_cache->cantidad_entradas > 0) {
    direccion_fisica = traducir_direccion(cpu,pid,nro_pagina,0);
    char* tam_pagina = string_itoa(tamanio_pagina);
    valor_a_leer = pedir_valor_a_memoria(cpu,pid, direccion_fisica, tam_pagina);
    free(tam_pagina);
  } else {
    direccion_fisica = traducir_direccion(cpu,pid,nro_pagina,desplazamiento);
    valor_a_leer = pedir_valor_a_memoria(cpu,pid, direccion_fisica, parametro2);
  }

  if (valor_a_leer == NULL) {
      log_debug(logger, "No se pudo leer de memoria");
      return;
  }
    
  //Log 4. Lectura Memoria
  log_info(logger, "PID: <%d> - Acción: <LEER> - Dirección Física: <%d> - Valor: <%s>", pid, direccion_fisica, valor_a_leer);

  if (cpu->parametros_cache->cantidad_entradas > 0) {
    actualizar_cache(cpu,pid,nro_pagina,valor_a_leer,es_escritura, desplazamiento);
  } else {
    free(valor_a_leer);
  }
}

void ejecutar_write (t_cpu* cpu, uint32_t pid, char* direccion_logica, char* valor_a_escribir){
  int direccion = atoi(direccion_logica);
  //Calcular numero de pagina y desplazamiento 
  uint32_t nro_pagina = floor(direccion / tamanio_pagina);
  uint32_t desplazamiento = direccion % tamanio_pagina;
  bool es_escritura=true;

  if (cpu->parametros_cache->cantidad_entradas > 0) {
    int ubicacion = consultar_pagina_cache(cpu,pid,nro_pagina);

    if (ubicacion != -1) {
      asignar_lugar_en_cache(cpu,ubicacion,pid,nro_pagina,valor_a_escribir,es_escritura,desplazamiento);
    }
    else {
      actualizar_cache(cpu,pid, nro_pagina,valor_a_escribir,es_escritura,desplazamiento);
    }
    log_info(logger, "PID: <%d> - Acción: <ESCRIBIR> - Valor: <%s>", pid, valor_a_escribir);
    return;
  }
  
  uint32_t direccion_fisica = traducir_direccion(cpu,pid,nro_pagina,desplazamiento);

  if (direccion_fisica==-1){
    log_debug(logger,"No se pudo obtener la dirección física, pid: %d",pid);
  }
    
  //Log 4. Lectura Memoria
  log_info(logger, "PID: <%d> - Acción: <ESCRIBIR> - Dirección Física: <%d> - Valor: <%s>", pid, direccion_fisica, valor_a_escribir);

  escribir_valor_en_memoria(cpu,pid, direccion_fisica, valor_a_escribir, ESCRITURA);
  
  if (cpu->parametros_cache->cantidad_entradas > 0) {
    actualizar_cache(cpu,pid, nro_pagina,valor_a_escribir,es_escritura, desplazamiento);
  }

}

void agregar_syscall_a_paquete(t_paquete* paquete, uint32_t pid, uint32_t tipo, char* arg1, char* arg2, uint32_t pc){
  agregar_a_paquete(paquete, &pid, sizeof(uint32_t));
  agregar_a_paquete(paquete, &tipo, sizeof(uint32_t));

  if(arg1 != NULL){
    uint32_t len = strlen(arg1) + 1;
    agregar_a_paquete(paquete, &len, sizeof(uint32_t));
    agregar_a_paquete(paquete, arg1, len);
  } else {
    uint32_t len = 0;
    agregar_a_paquete(paquete, &len, sizeof(uint32_t));
  };
  if(arg2 != NULL){
    uint32_t len = strlen(arg2) + 1;
    agregar_a_paquete(paquete, &len, sizeof(uint32_t));
    agregar_a_paquete(paquete, arg2, len);
  } else {
    uint32_t len = 0;
    agregar_a_paquete(paquete, &len, sizeof(uint32_t));
  };
  agregar_a_paquete(paquete, &pc, sizeof(uint32_t));
};

//Retornar a Kernel el PCB actualizado y el motivo de la interrupción
void actualizar_kernel(t_instruccion instruccion, t_estado_ejecucion estado, t_pcb* pcb, int conexion_kernel_dispatch){
  t_paquete* paquete = NULL;
  switch(estado){
    case EJECUCION_CONTINUA_INIT_PROC:
      paquete = crear_paquete(SYSCALL);
      agregar_syscall_a_paquete(paquete, pcb->pid, SYSCALL_INIT_PROC, instruccion.parametro1, instruccion.parametro2, pcb->pc);
      free(instruccion.parametro1);
      free(instruccion.parametro2);
      break;
    
    case EJECUCION_FINALIZADA:
      paquete = crear_paquete(SYSCALL);
      agregar_syscall_a_paquete(paquete, pcb->pid, SYSCALL_EXIT, "", "", pcb->pc);
      break;

    case EJECUCION_BLOQUEADA_IO:
      paquete = crear_paquete(SYSCALL);
      agregar_syscall_a_paquete(paquete, pcb->pid, SYSCALL_IO, instruccion.parametro1, instruccion.parametro2, pcb->pc);
      free(instruccion.parametro1);
      free(instruccion.parametro2);
      break;

    case EJECUCION_BLOQUEADA_DUMP:
      paquete = crear_paquete(SYSCALL);
      agregar_syscall_a_paquete(paquete, pcb->pid, SYSCALL_DUMP_MEMORY, "", "", pcb->pc);
      break;

    case EJECUCION_DESALOJADO:
      paquete = crear_paquete(DESALOJO);
      agregar_a_paquete(paquete, (&pcb->pid), sizeof(uint32_t));
      agregar_a_paquete(paquete, (&pcb->pc), sizeof(uint32_t));
      break;

    default:
      log_warning(logger, "Motivo de interrupción desconocida. Estado: <%d>", estado);
      return;
  };
  enviar_paquete(paquete, conexion_kernel_dispatch);
};

//funcion que espera el mensaje de kernel
bool chequear_interrupcion(int socket_interrupt, uint32_t pid_actual) {
  int pid_interrupcion;
  int bytes = recv(socket_interrupt, &pid_interrupcion, sizeof(uint32_t), MSG_DONTWAIT);

  if (bytes > 0) {
    //Log 2. Interrupción recibida
    log_info(logger, "## Llega interrupción al puerto Interrupt");
    if(pid_interrupcion == pid_actual){
      return true;
    }
  }
  return false;
}

