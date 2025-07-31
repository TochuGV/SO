#include "syscalls.h"

t_syscall* recibir_syscall(int socket_cliente){
  t_list* lista = recibir_paquete(socket_cliente);
  if(!lista || list_size(lista) < 6){
    log_error(logger, "Error al recibir la llamada al sistema: Lista vacía o incompleta");
    return NULL;
  };

  t_syscall* syscall = malloc(sizeof(t_syscall));
  int offset = 0;

  syscall->pid = *(uint32_t*) list_get(lista, offset++);
  syscall->tipo = *(uint32_t*) list_get(lista, offset++);

  uint32_t arg1_len = *(uint32_t*) list_get(lista, offset++);
  syscall->arg1 = NULL;
  if(arg1_len > 0){
    syscall->arg1 = strdup((char*) list_get(lista, offset++));
  };

  uint32_t arg2_len = *(uint32_t*) list_get(lista, offset++);
  syscall->arg2 = NULL;
  if(arg2_len > 0){
    syscall->arg2 = strdup((char*) list_get(lista, offset++));
  };

  syscall->pc = *(uint32_t*) list_get(lista, offset++);

  list_destroy_and_destroy_elements(lista, free);
  return syscall;
};

void syscall_init_proc(t_syscall* syscall){
  t_pcb* pcb_nuevo = crear_pcb();
  char* archivo_pseudocodigo = strdup(syscall->arg1);
  uint32_t tamanio = atoi(syscall->arg2);
  inicializar_proceso(pcb_nuevo, archivo_pseudocodigo, tamanio);
};

void syscall_exit(t_syscall* syscall){
  t_pcb* pcb = obtener_pcb_por_pid(syscall->pid);
  if(pcb == NULL) return;
  cambiar_estado(pcb, ESTADO_EXEC, ESTADO_EXIT);
  liberar_cpu_por_pid(pcb->pid);
  finalizar_proceso(pcb);
};

void syscall_io(t_syscall* syscall){
  t_pcb* pcb = obtener_pcb_por_pid(syscall->pid);
  if(pcb == NULL){
    log_warning(logger, "No existe el PCB");
    return;
  };
  pcb->pc = syscall->pc;

  char* clave_pid = string_itoa(pcb->pid);
  
  if(strcmp(ALGORITMO_CORTO_PLAZO, "SJF") == 0 || strcmp(ALGORITMO_CORTO_PLAZO, "SRT") == 0){

    pthread_mutex_lock(&mutex_diccionario_cronometros);
    t_temporizadores_estado* tiempos = dictionary_get(diccionario_cronometros, clave_pid);
    pthread_mutex_unlock(&mutex_diccionario_cronometros);

    if(tiempos && tiempos->cronometros_estado[ESTADO_EXEC]){
      double rafaga_real = temporal_gettime(tiempos->cronometros_estado[ESTADO_EXEC]);
      actualizar_estimacion(pcb->pid, rafaga_real);
      log_debug(logger, "PID <%d> - Ráfaga real: %.2f - Estimación actualizada", pcb->pid, rafaga_real);
    };
  };
  pthread_mutex_lock(&mutex_diccionario_dispositivos);
  t_list* lista_instancias = dictionary_get(diccionario_dispositivos, syscall->arg1);
  pthread_mutex_unlock(&mutex_diccionario_dispositivos);
  if(!lista_instancias || list_size(lista_instancias) == 0){
    log_error(logger, "No hay instancias disponibles para el dispositivo <%s>. Proceso <%d> finalizando...", syscall->arg1, pcb->pid);
    cambiar_estado(pcb, ESTADO_EXEC, ESTADO_EXIT);
    liberar_cpu_por_pid(pcb->pid);
    finalizar_proceso(pcb);
    return;
  };

  t_instancia_io* instancia_libre = NULL; //Buscar instancia libre
  for(int i = 0; i < list_size(lista_instancias); i++){
    t_instancia_io* instancia = list_get(lista_instancias, i);
    if(!instancia->ocupado){
      instancia_libre = instancia;
      break;
    };
  };

  t_contexto_io* contexto = malloc(sizeof(t_contexto_io));
  contexto->dispositivo_actual = strdup(syscall->arg1);
  contexto->duracion_io = atoi(syscall->arg2);
  pthread_mutex_lock(&mutex_diccionario_contextos);
  dictionary_put(diccionario_contextos_io, clave_pid, contexto);
  pthread_mutex_unlock(&mutex_diccionario_contextos);
  free(clave_pid);
  cambiar_estado(pcb, ESTADO_EXEC, ESTADO_BLOCKED);
  log_motivo_bloqueo(pcb->pid, contexto->dispositivo_actual);

  if(instancia_libre){
    instancia_libre->ocupado = true;
    enviar_peticion_io(instancia_libre->socket, contexto->duracion_io, pcb->pid);
  } else {
    t_instancia_io* menos_cargada = list_get(lista_instancias, 0);
    for(int i = 1; i < list_size(lista_instancias); i++){
      t_instancia_io* actual = list_get(lista_instancias, i);
      if(queue_size(actual->cola_bloqueados) < queue_size(menos_cargada->cola_bloqueados)){
        menos_cargada = actual;
      };
    };

    queue_push(menos_cargada->cola_bloqueados, pcb);
  };

  if (contexto->duracion_io >= TIEMPO_SUSPENSION) {
    uint32_t* pid_arg = malloc(sizeof(uint32_t));
    *pid_arg = pcb->pid;
    pthread_t hilo_revisar_bloqueados;
    pthread_create(&hilo_revisar_bloqueados, NULL, revisar_bloqueados, pid_arg);
    pthread_detach(hilo_revisar_bloqueados);
  }
    //sem_post(&semaforo_revisar_bloqueados);

  liberar_cpu_por_pid(pcb->pid);
};

void syscall_dump_memory(t_syscall* syscall){ // Se pide un volcado de información de un proceso obtenido por el PID.
  t_pcb* pcb = obtener_pcb_por_pid(syscall->pid);
  if(pcb == NULL){
    log_warning(logger, "No existe el PCB");
    return;
  };
  pcb->pc = syscall->pc;

  t_paquete* paquete = crear_paquete(SOLICITUD_DUMP_MEMORY);
  agregar_a_paquete(paquete, &(pcb->pid), sizeof(uint32_t));

  pthread_mutex_lock(&mutex_memoria);
  int socket_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA, MODULO_KERNEL);
  if(handshake_kernel(socket_memoria) != 0){
    log_error(logger, "No se pudo conectar a Memoria para solicitar DUMP_MEMORY");
    return;
  };

  enviar_paquete(paquete, socket_memoria);
  
  int respuesta = recibir_operacion(socket_memoria);
  close(socket_memoria);
  pthread_mutex_unlock(&mutex_memoria);

  cambiar_estado(pcb, ESTADO_EXEC, ESTADO_BLOCKED);
  liberar_cpu_por_pid(pcb->pid);

  if(respuesta == 0){
    encolar_proceso_en_ready(pcb);
    cambiar_estado(pcb, ESTADO_BLOCKED, ESTADO_READY);
  } else {
    log_debug(logger, "Error al realizar volcado de memoria para proceso <%d>", pcb->pid);
    cambiar_estado(pcb, ESTADO_BLOCKED, ESTADO_EXIT);
    finalizar_proceso(pcb);
  };
};

void manejar_syscall(t_syscall* syscall, int socket_cpu_dispatch){
  log_syscall_recibida(syscall->pid, syscall->tipo);
  switch(syscall->tipo){
    case SYSCALL_INIT_PROC:
      syscall_init_proc(syscall);
      break;
    case SYSCALL_EXIT:
      syscall_exit(syscall);
      break;
    case SYSCALL_IO:
      syscall_io(syscall);
      break;
    case SYSCALL_DUMP_MEMORY:
      syscall_dump_memory(syscall);
      break;
    default:
      log_warning(logger, "Syscall desconocida");
  };
};

void destruir_syscall(t_syscall* syscall){
  if(!syscall) return;
  if(syscall->arg1) free(syscall->arg1);
  if(syscall->arg2) free(syscall->arg2);
  free(syscall);
};