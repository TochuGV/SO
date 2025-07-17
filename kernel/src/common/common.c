#include "common.h"

char* NOMBRES_DISPOSITIVOS_IO[] = { "IMPRESORA", "TECLADO", "MOUSE", "AURICULARES", "PARLANTE", "DISCO" };
char* NOMBRES_SYSCALLS[] = { "INIT_PROC", "EXIT", "IO", "DUMP_MEMORY" };

uint32_t enviar_proceso_a_memoria(char* archivo_pseudocodigo, uint32_t tamanio_proceso, uint32_t pid){

  t_paquete* paquete = crear_paquete(SOLICITUD_MEMORIA);
  int32_t resultado;
  if(archivo_pseudocodigo != NULL){
    uint32_t longitud_archivo_pseudocodigo = strlen(archivo_pseudocodigo) + 1;
    agregar_a_paquete(paquete, &longitud_archivo_pseudocodigo, sizeof(uint32_t));
    agregar_a_paquete(paquete, archivo_pseudocodigo, longitud_archivo_pseudocodigo);
    agregar_a_paquete(paquete, &tamanio_proceso, sizeof(tamanio_proceso));
    agregar_a_paquete(paquete, &pid, sizeof(uint32_t));

    pthread_mutex_lock(&mutex_memoria);
    int socket_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA, MODULO_KERNEL);
    if(handshake_kernel(socket_memoria) != 0){
      log_error(logger, "Handshake fallido con Memoria para el proceso <%d>", pid);
      close(socket_memoria);
      return -1;
    };
    enviar_paquete(paquete, socket_memoria);
    recv(socket_memoria, &resultado, sizeof(int32_t), MSG_WAITALL);
    close(socket_memoria);
    pthread_mutex_unlock(&mutex_memoria);

    if(resultado == 0){
      return 0;
    } else {
      //log_warning(logger, "Memoria rechazó el proceso <%d> debido a falta de espacio u otro motivo", pid);
      return -1;
    }
  } else {
    uint32_t longitud_archivo_pseudocodigo = 0;
    agregar_a_paquete(paquete, &longitud_archivo_pseudocodigo, sizeof(uint32_t));
    //log_warning(logger, "Archivo pseudocodigo es NULL, temporalmente. Se envía como longitud '0'");
    pthread_mutex_lock(&mutex_memoria);
    int socket_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA, MODULO_KERNEL);
    if(handshake_kernel(socket_memoria) != 0){
      log_error(logger, "Handshake fallido con Memoria para el proceso <%d>", pid);
      close(socket_memoria);
      return -1;
    };
    enviar_paquete(paquete, socket_memoria);
    close(socket_memoria);
    pthread_mutex_unlock(&mutex_memoria);
    return -1;
  };
};

void esperar_enter_para_planificar(){
  char* leido = readline("");
  if(leido != NULL && strlen(leido) == 0){
    free(leido);
    return;
  };
  while(leido == NULL || strlen(leido) != 0){
    free(leido);
    leido = readline("");
  };
  free(leido);
};

char* crear_cadena_metricas_estado(t_pcb* pcb){
  char* buffer = string_from_format("## (<%d>) - Métricas de estado: ", pcb->pid);
  for(int i = 0; i < CANTIDAD_ESTADOS - 1; i++){
    char* aux = string_from_format("%s (%d) (%d)", obtener_nombre_estado(i), pcb->me[i], pcb->mt[i]);
    string_append(&buffer, aux);
    if(i < CANTIDAD_ESTADOS - 2) string_append(&buffer, ", ");
    free(aux);
  };
  return buffer;
};

char* token_io_to_string(int32_t token) {
  switch (token) {
    case IMPRESORA: return "IMPRESORA";
    case TECLADO: return "TECLADO";
    case MOUSE: return "MOUSE";
    case AURICULARES: return "AURICULARES";
    case PARLANTE: return "PARLANTE";
    case DISCO: return "DISCO";
    default: return NULL;
  };
};

t_pcb* obtener_pcb_por_pid(uint32_t pid){
  pthread_mutex_lock(&mutex_pcbs);
  bool coincide(void* elem){
    return ((t_pcb*)elem)->pid == pid;
  };
  t_pcb* pcb = list_find(lista_pcbs, coincide);
  pthread_mutex_unlock(&mutex_pcbs);
  return pcb;
};

void destruir_contexto_io(void* contexto){
  t_contexto_io* ctx = (t_contexto_io*) contexto;
  if(ctx->dispositivo_actual) free(ctx->dispositivo_actual);
  free(ctx);
};

void encolar_proceso_en_ready(t_pcb* pcb){
  pthread_mutex_lock(&mutex_ready);
  queue_push(cola_ready, pcb);
  pthread_mutex_unlock(&mutex_ready);
  if(es_SRT()) desalojar_cpu(pcb);
  sem_post(&semaforo_ready);
};