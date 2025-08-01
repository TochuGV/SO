#include "io.h"

void* conectar_io(void* arg){
  aceptar_conexiones((char*) arg, atender_io, "IO");
  return NULL;
};

void* atender_io(void* arg){
  int socket_io = *(int*)arg;
  free(arg);
  
  if(!validar_handshake_cliente(socket_io, MODULO_IO, "IO"))
    pthread_exit(NULL);

  t_list* lista;
  while (1) {
    int cod_op = recibir_operacion(socket_io);
    switch (cod_op){
      case PAQUETE:
        lista = recibir_paquete(socket_io);
        list_iterate(lista, (void*) iterator);
        list_destroy_and_destroy_elements(lista, free);
        break;
      case FINALIZACION_IO:
        lista = recibir_paquete(socket_io);
        if(list_size(lista) == 0){
          log_error(logger, "Paquete FINALIZACION_IO inválido");
          break;
        } else {
          uint32_t pid = *(uint32_t*) list_get(lista, 0);
          manejar_respuesta_io(pid, socket_io);
          list_destroy_and_destroy_elements(lista, free);
        };
        break;
      case -1:
        log_warning(logger, "Dispositivo IO desconectado");

        for(int i = 0; i < 6; i++){
          pthread_mutex_lock(&mutex_diccionario_dispositivos);
          t_list* lista_instancias = dictionary_get(diccionario_dispositivos, NOMBRES_DISPOSITIVOS_IO[i]);
          pthread_mutex_unlock(&mutex_diccionario_dispositivos);
    
          for(int j = list_size(lista_instancias) -1 ; j >= 0; j--){
            t_instancia_io* instancia = list_get(lista_instancias, j);
            if(instancia->socket == socket_io){
              if(instancia->pcb_bloqueado){
                  cambiar_estado(instancia->pcb_bloqueado, ESTADO_BLOCKED, ESTADO_EXIT);
                  finalizar_proceso(instancia->pcb_bloqueado);
                };

                if(!queue_is_empty(instancia->cola_bloqueados)) {
                  if(list_size(lista_instancias) == 1) {
                    while(!queue_is_empty(instancia->cola_bloqueados)) {
                      t_pcb* pcb = queue_pop(instancia->cola_bloqueados);
                      log_error(logger, "No hay instancias disponibles para el dispositivo <%s>. Proceso <%d> finalizando...", NOMBRES_DISPOSITIVOS_IO[i], pcb->pid);
                      cambiar_estado(pcb, ESTADO_BLOCKED, ESTADO_EXIT);
                      finalizar_proceso(pcb);
                    };
                  } else {
                    for(int k = 0; k < list_size(lista_instancias); k++) {
                      t_instancia_io* instancia_secundaria = list_get(lista_instancias, k);
                      if(instancia_secundaria->socket != socket_io){ 
                        while(!queue_is_empty(instancia->cola_bloqueados)) {
                          t_pcb* pcb = queue_pop(instancia->cola_bloqueados);
                          queue_push(instancia_secundaria->cola_bloqueados, pcb);
                        };
                      };
                    };
                  };
                };
              list_remove_and_destroy_element(lista_instancias, j, free);
            };
          };
        };

        close(socket_io);
        pthread_exit(NULL);
      default:
        log_warning(logger, "Operación desconocida desde IO: %d", cod_op);
        break;
    };
  };
  close(socket_io);
  pthread_exit(NULL);
};

int comparar_por_tiempo_bloqueado(void* a, void* b) {
  t_pcb* pcb1 = (t_pcb*) a;
  t_pcb* pcb2 = (t_pcb*) b;

  char* clave1 = string_itoa(pcb1->pid);
  char* clave2 = string_itoa(pcb2->pid);

  t_temporizadores_estado* tiempos1 = dictionary_get(diccionario_cronometros, clave1);
  t_temporizadores_estado* tiempos2 = dictionary_get(diccionario_cronometros, clave2);

  double t1 = tiempos1 && tiempos1->cronometros_estado[ESTADO_BLOCKED] ? temporal_gettime(tiempos1->cronometros_estado[ESTADO_BLOCKED]) : 0;
  double t2 = tiempos2 && tiempos2->cronometros_estado[ESTADO_BLOCKED] ? temporal_gettime(tiempos2->cronometros_estado[ESTADO_BLOCKED]) : 0;

  free(clave1);
  free(clave2);
  return t1 < t2 ? -1 : (t1 > t2 ? 1 : 0);
};

void registrar_socket_io(char* nombre, int socket){
  if(!diccionario_dispositivos){
    log_error(logger, "El diccionario de dispositivos IO no está inicializado.");
    return;
  };

  pthread_mutex_lock(&mutex_diccionario_dispositivos);
  t_list* lista_instancias = dictionary_get(diccionario_dispositivos, nombre);
  pthread_mutex_unlock(&mutex_diccionario_dispositivos);
  
  if(!lista_instancias){
    log_error(logger, "No existe el dispositivo IO: %s", nombre);
    return;
  };

  t_instancia_io* nueva_instancia = malloc(sizeof(t_instancia_io));
  nueva_instancia->socket = socket;
  nueva_instancia->cola_bloqueados = queue_create();
  nueva_instancia->pcb_bloqueado = NULL;
  list_add(lista_instancias, nueva_instancia);

  t_list* lista_bloqueados = list_create();
  for(int i = 0; i < list_size(lista_instancias); i++){
    t_instancia_io* instancia = list_get(lista_instancias, i);

    if(instancia != nueva_instancia){
      while(!queue_is_empty(instancia->cola_bloqueados)){
        t_pcb* pcb = queue_pop(instancia->cola_bloqueados);
        list_add(lista_bloqueados, pcb);
      };
    };
  };

  list_sort(lista_bloqueados, (void*) comparar_por_tiempo_bloqueado);

  int cantidad_instancias = list_size(lista_instancias);
  for(int i = 0; i < list_size(lista_bloqueados); i++){
    t_pcb* pcb = list_get(lista_bloqueados, i);
    t_instancia_io* instancia_destino = list_get(lista_instancias, i % cantidad_instancias);
    queue_push(instancia_destino->cola_bloqueados, pcb);
  };

  list_destroy(lista_bloqueados);
};

void iniciar_instancia_io_si_corresponde(char* nombre, int socket){
  pthread_mutex_lock(&mutex_diccionario_dispositivos);
  t_list* lista_instancias = dictionary_get(diccionario_dispositivos, nombre);
  pthread_mutex_unlock(&mutex_diccionario_dispositivos);

  if(!lista_instancias) return;

  t_instancia_io* instancia = NULL;
  for(int i = 0; i < list_size(lista_instancias); i++){
    t_instancia_io* actual = list_get(lista_instancias, i);
    if(actual->socket == socket){
      instancia = actual;
      break;
    };
  };
  
  if(instancia && !instancia->pcb_bloqueado && !queue_is_empty(instancia->cola_bloqueados)){
    t_pcb* siguiente = queue_pop(instancia->cola_bloqueados);
    char* clave_pid = string_itoa(siguiente->pid);
    pthread_mutex_lock(&mutex_diccionario_contextos);
    t_contexto_io* contexto = dictionary_get(diccionario_contextos_io, clave_pid);
    pthread_mutex_unlock(&mutex_diccionario_contextos);
    free(clave_pid);

    if(contexto){
      enviar_peticion_io(instancia->socket, contexto->duracion_io, siguiente->pid);
      instancia->pcb_bloqueado = siguiente;
    };
  };
}

void enviar_peticion_io(int socket_io, uint32_t duracion, uint32_t pid){
  t_paquete* paquete = crear_paquete(PETICION_IO);
  agregar_a_paquete(paquete, &pid, sizeof(uint32_t));
  agregar_a_paquete(paquete, &duracion, sizeof(uint32_t));
  enviar_paquete(paquete, socket_io);
};