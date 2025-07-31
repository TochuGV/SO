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
              if(instancia->ocupado) {
                t_pcb* pcb = queue_pop(instancia->cola_bloqueados);
                cambiar_estado(pcb, ESTADO_BLOCKED, ESTADO_EXIT);
                finalizar_proceso(pcb);

                if(!queue_is_empty(instancia->cola_bloqueados)) {
                  if(list_size(lista_instancias) == 1) {
                    while(!queue_is_empty(instancia->cola_bloqueados)) {
                      t_pcb* pcb = queue_pop(instancia->cola_bloqueados);
                      log_error(logger, "No hay instancias disponibles para el dispositivo <%s>. Proceso <%d> finalizando...", NOMBRES_DISPOSITIVOS_IO[i], pcb->pid);
                      cambiar_estado(pcb, ESTADO_BLOCKED, ESTADO_EXIT);
                      finalizar_proceso(pcb);
                    }
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
                }
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

void registrar_socket_io(char* nombre, int socket){
  if(!diccionario_dispositivos){
    log_error(logger, "El diccionario de dispositivos IO no está inicializado.");
    return;
  };

  t_list* lista_instancias = dictionary_get(diccionario_dispositivos, nombre);
  if(!lista_instancias){
    log_error(logger, "No existe el dispositivo IO: %s", nombre);
    return;
  };

  t_instancia_io* nueva_instancia = malloc(sizeof(t_instancia_io));
  nueva_instancia->socket = socket;
  nueva_instancia->ocupado = false;
  nueva_instancia->cola_bloqueados = queue_create();
  list_add(lista_instancias, nueva_instancia);
};

void enviar_peticion_io(int socket_io, uint32_t duracion, uint32_t pid){
  t_paquete* paquete = crear_paquete(PETICION_IO);
  agregar_a_paquete(paquete, &pid, sizeof(uint32_t));
  agregar_a_paquete(paquete, &duracion, sizeof(uint32_t));
  enviar_paquete(paquete, socket_io);
};