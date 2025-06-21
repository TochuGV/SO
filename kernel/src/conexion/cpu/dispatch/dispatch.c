#include "dispatch.h"

void* conectar_cpu_dispatch(void* arg){
  char* puerto = (char*) arg;
  int socket_escucha = iniciar_servidor(puerto);
  log_info(logger, "Esperando conexiones de CPU Dispatch...");
  while(1){
    int socket_cliente = esperar_cliente(socket_escucha);
    if(socket_cliente == -1){
      log_error(logger, "Error al aceptar conexión desde CPU Dispatch");
      continue;
    };
    int* socket_ptr = malloc(sizeof(int));
    *socket_ptr = socket_cliente;
    pthread_t hilo_atender_cpu_dispatch;
    pthread_create(&hilo_atender_cpu_dispatch, NULL, atender_cpu_dispatch, socket_ptr);
    pthread_detach(hilo_atender_cpu_dispatch);
  };
  return NULL;
};

void* atender_cpu_dispatch(void* arg){
  int socket_cpu_dispatch = *(int*) arg;
  free(arg);
  if(recibir_handshake_kernel(socket_cpu_dispatch) != MODULO_CPU){
    log_error(logger, "Se esperaba un CPU, pero se conectó otro tipo");
    close(socket_cpu_dispatch);
    pthread_exit(NULL);
  };
  log_info(logger, "CPU Dispatch conectada correctamente");
  while(1){
    int cod_op = recibir_operacion(socket_cpu_dispatch);
    if(cod_op == -1){
      log_warning(logger, "CPU Dispatch desconectada");
      break;
    };
    //log_debug(logger, "Código de operación recibido: %d", cod_op);
    
    if(cod_op == DESALOJO){
      t_list* lista = recibir_paquete(socket_cpu_dispatch);
      if(!lista || list_size(lista) == 0){
        log_error(logger, "Fallo al recibir PCB desalojado");
        break;
      } else {
        uint32_t pid = *(uint32_t*) list_get(lista, 0);
        uint32_t pc = *(uint32_t*) list_get(lista, 1);

        list_destroy_and_destroy_elements(lista, free);
        
        log_desalojo_sjf_srt(pid);

        t_pcb* pcb = obtener_pcb_por_pid(pid);
        pcb->pc = pc;

        encolar_proceso_en_ready(pcb);
        cambiar_estado(pcb, ESTADO_EXEC, ESTADO_READY);


        // Liberar la CPU correspondiente (ya no está ejecutando)
        pthread_mutex_lock(&mutex_cpus);
        for (int i = 0; i < list_size(lista_cpus); i++) {
          t_cpu* cpu = list_get(lista_cpus, i);
          if (!cpu->disponible && cpu->proceso_en_ejecucion && cpu->proceso_en_ejecucion->pid == pcb->pid) {
            cpu->proceso_en_ejecucion = NULL;
            cpu->disponible = true;
            break;
          }
        }
        pthread_mutex_unlock(&mutex_cpus);

        sem_post(&semaforo_ready);
        sem_post(&semaforo_cpu_libre);
        continue;
      }
    }

    t_syscall* syscall = recibir_syscall(socket_cpu_dispatch);
    if(!syscall){
      log_error(logger, "Fallo al recibir la llamada al sistema. Cerrando la conexión con CPU Dispatch...");
      break;
    }
    manejar_syscall(syscall, socket_cpu_dispatch);
    destruir_syscall(syscall);
  };
  close(socket_cpu_dispatch);
  return NULL;
};