#include "common.h"

char* NOMBRES_DISPOSITIVOS_IO[] = { "Impresora", "Teclado", "Mouse", "Auriculares", "Parlante" };
char* NOMBRES_SYSCALLS[] = { "INIT_PROC", "EXIT", "IO", "DUMP_MEMORY" };

uint32_t enviar_proceso_a_memoria(char* archivo_pseudocodigo, uint32_t tamanio_proceso, uint32_t pid, int socket_cliente){
  t_paquete* paquete = crear_paquete(SOLICITUD_MEMORIA);
  uint32_t longitud_archivo_pseudocodigo;
  int32_t resultado;
  if(archivo_pseudocodigo != NULL){
    longitud_archivo_pseudocodigo = strlen(archivo_pseudocodigo) + 1;
    agregar_a_paquete(paquete, &longitud_archivo_pseudocodigo, sizeof(uint32_t));// Enviar la longitud
    agregar_a_paquete(paquete, archivo_pseudocodigo, longitud_archivo_pseudocodigo);
    agregar_a_paquete(paquete, &tamanio_proceso, sizeof(tamanio_proceso));
    agregar_a_paquete(paquete, &pid, sizeof(uint32_t));
    enviar_paquete(paquete, socket_cliente);
    log_info(logger, "Archivo pseudocódigo '%s' enviado a memoria para el proceso <%d>", archivo_pseudocodigo, pid);
    recv(socket_cliente, &resultado, sizeof(int32_t), MSG_WAITALL);
    if(resultado == 0){
      log_debug(logger, "Memoria aceptó el proceso <%d>", pid);
      return 0;
    } else {
      log_warning(logger, "Memoria rechazó el proceso <%d> debido a falta de espacio u otro motivo", pid);
      return -1;
    }
  } else {
    longitud_archivo_pseudocodigo = 0;
    agregar_a_paquete(paquete, &longitud_archivo_pseudocodigo, sizeof(uint32_t));
    log_warning(logger, "Archivo pseudocodigo es NULL, temporalmente. Se envía como longitud '0'");
    enviar_paquete(paquete, socket_cliente);
  };
  return -1;
};

void esperar_enter_para_planificar(){ //Se podría ver alguna forma de que se loggee el mensaje "Presiona 'Enter' para comenzar la planificación..."
  char* leido = readline("Presiona 'Enter' para comenzar la planificación...\n");
  if(leido != NULL && strlen(leido) == 0){
    log_debug(logger, "Se presionó 'Enter', comenzando la planificación...");
    free(leido);
    return;
  };
  while(leido == NULL || strlen(leido) != 0){
    free(leido);
    leido = readline("Presiona solo 'Enter' para comenzar la planificación...\n");
  };
  log_debug(logger, "Se presionó 'Enter', comenzando la planificación...");
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
    case IMPRESORA: return "Impresora";
    case TECLADO: return "Teclado";
    case MOUSE: return "Mouse";
    case AURICULARES: return "Auriculares";
    case PARLANTE: return "Parlante";
    default: return NULL;
  };
};

t_pcb* obtener_pcb_por_pid(uint32_t pid){
  pthread_mutex_lock(&mutex_pcbs);
  bool coincide(void* elem){
    return ((t_pcb*)elem)->pid == pid;
  };
  log_debug(logger, "%d", list_size(lista_pcbs));
  t_pcb* pcb = list_find(lista_pcbs, coincide);
  log_debug(logger, "PCB encontrado PID: %d", pcb->pid);
  pthread_mutex_unlock(&mutex_pcbs);
  return pcb;
};