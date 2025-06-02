#include "common.h"

uint32_t enviar_proceso_a_memoria(char* archivo_pseudocodigo, uint32_t tamanio_proceso, uint32_t pid, int socket_cliente){
  /*
  t_paquete* paquete = crear_paquete(SOLICITUD_MEMORIA);
  uint32_t longitud_archivo_pseudocodigo;
  uint32_t resultado;
  if(archivo_pseudocodigo != NULL){
    longitud_archivo_pseudocodigo = strlen(archivo_pseudocodigo) + 1;
    agregar_a_paquete(paquete, &longitud_archivo_pseudocodigo, sizeof(uint32_t));// Enviar la longitud
    agregar_a_paquete(paquete, archivo_pseudocodigo, longitud_archivo_pseudocodigo);
    agregar_a_paquete(paquete, &tamanio_proceso, sizeof(tamanio_proceso));
    agregar_a_paquete(paquete, &pid, sizeof(uint32_t));

    enviar_paquete(paquete, socket_cliente);

    log_info(logger, "Archivo pseudocódigo enviado a memoria!");

    recv(socket_cliente, &resultado, sizeof(uint32_t), MSG_WAITALL);

    if (resultado == 0)
      return 0;
    else
      return -1;    
  } else {
    longitud_archivo_pseudocodigo = 0;
    agregar_a_paquete(paquete, &longitud_archivo_pseudocodigo, sizeof(uint32_t));
    log_warning(logger, "Archivo pseudocodigo es NULL, temporalmente. Se envía como longitud '0'");
    enviar_paquete(paquete, socket_cliente);
  };
  return -1;
  */
  log_warning(logger, "Omitiendo el envío real a Memoria - Proceso <%d>", pid);
  return 0;
};

char* crear_cadena_metricas_estado(t_pcb* pcb){
  char* buffer = string_from_format("## (<%d>) - Métricas de estado: ", pcb->pid);
  for(int i = 0; i < CANTIDAD_ESTADOS; i++){
    char* aux = string_from_format("%s (%d) (%d)", obtener_nombre_estado(i), pcb->me[i], pcb->mt[i]);
    string_append(&buffer, aux);
    if(i < CANTIDAD_ESTADOS - 1) string_append(&buffer, ", ");
    free(aux);
  };
  return buffer;
};

char* NOMBRES_SYSCALLS[] = {
  "IO",
  "EXIT"
  "INIT_PROC",
  "DUMP_MEMORY",
};

char* NOMBRES_DISPOSITIVOS_IO[] = {
  "IMPRESORA",
  "TECLADO",
  "MOUSE",
  "AURICULARES",
  "PARLANTE"
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
  t_pcb* pcb = list_find(lista_pcbs, coincide);
  pthread_mutex_unlock(&mutex_pcbs);
  return pcb;
};