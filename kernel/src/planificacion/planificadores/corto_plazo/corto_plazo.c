#include "corto_plazo.h"
#include "planificacion/algoritmos/sjf/sjf.h"

t_queue* cola_ready;
sem_t semaforo_ready;
pthread_mutex_t mutex_ready = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cpus = PTHREAD_MUTEX_INITIALIZER;

void iniciar_planificacion_corto_plazo() {
  cola_ready = queue_create();
  sem_init(&semaforo_ready, 0, 0);
  pthread_mutex_init(&mutex_ready, NULL);
  pthread_mutex_init(&mutex_cpus, NULL);
};

void mover_proceso_a_exec(){
  pthread_mutex_lock(&mutex_ready);

  log_debug(logger, "Estimaciones actuales de procesos en READY:");
  t_list* lista_auxiliar = list_create();

  while (!queue_is_empty(cola_ready)) {
    t_pcb* pcb = queue_pop(cola_ready);
    list_add(lista_auxiliar, pcb);

    char* clave = string_itoa(pcb->pid);
    double* estimacion = dictionary_get(diccionario_estimaciones, clave);
    double valor = estimacion ? *estimacion : ESTIMACION_INICIAL;
    log_debug(logger, "PID <%d> - Estimación: %.2f", pcb->pid, valor);
    free(clave);
  }

  // Volvemos a poner los procesos en la cola
  for (int i = 0; i < list_size(lista_auxiliar); i++) {
    t_pcb* pcb = list_get(lista_auxiliar, i);
    queue_push(cola_ready, pcb);
  }
  list_destroy(lista_auxiliar);

  if (queue_is_empty(cola_ready)) { 
    log_info(logger, "No hay procesos en la cola READY"); 
    pthread_mutex_unlock(&mutex_ready); 
    return;
  };
  
  t_pcb* pcb_elegido = obtener_proximo_proceso_ready(cola_ready);
  /*
  if (strcmp(ALGORITMO_CORTO_PLAZO, "SJF") == 0) {
    double estimacion = obtener_estimacion(pcb_elegido->pid);
    log_info(logger, "SJF seleccionó PID <%d> con estimación %.2f", pcb_elegido->pid, estimacion);
  }
  */
  pthread_mutex_unlock(&mutex_ready);
  pthread_mutex_lock(&mutex_cpus);
  t_cpu* cpu_seleccionada = NULL;
  for(int i = 0; i < list_size(lista_cpus); i++){
    t_cpu* cpu_actual = list_get(lista_cpus, i);
    if(cpu_actual->disponible){
      cpu_seleccionada = cpu_actual;
      cpu_actual->disponible = false;
      break;
    };
  };

  pthread_mutex_unlock(&mutex_cpus);
  if(cpu_seleccionada == NULL){
    log_info(logger, "No hay CPUs disponibles. Esperando...");
    encolar_proceso_en_ready(pcb_elegido);
    return;
  };

  log_info(logger, "Asignando proceso %d a CPU %d", pcb_elegido->pid, cpu_seleccionada->id_cpu);
  cpu_seleccionada->proceso_en_ejecucion = pcb_elegido; 
  enviar_a_cpu(pcb_elegido, cpu_seleccionada->socket_dispatch);
  cambiar_estado(pcb_elegido, ESTADO_READY, ESTADO_EXEC);
};

void enviar_a_cpu(t_pcb* pcb, int socket_cpu_dispatch){
  int bytes;
  void* stream = serializar_pcb_para_cpu(pcb, &bytes);
  t_paquete* paquete = crear_paquete(PCB);
  agregar_a_paquete(paquete, stream, bytes);
  enviar_paquete(paquete, socket_cpu_dispatch);
  free(stream);
};

void liberar_cpu(uint32_t id_cpu) {
  for(int i = 0; i < list_size(lista_cpus); i++) {
    t_cpu* cpu_actual = list_get(lista_cpus, i);
    if(cpu_actual->id_cpu == id_cpu){
      cpu_actual->disponible = true;
      break;
    };
  };
};

void liberar_cpu_por_pid(uint32_t pid){
  pthread_mutex_lock(&mutex_cpus);
  for(int i = 0; i < list_size(lista_cpus); i++){
    t_cpu* cpu = list_get(lista_cpus, i);
    if(!cpu->disponible){
      cpu->disponible = true;
      cpu->proceso_en_ejecucion = NULL;
      sem_post(&semaforo_cpu_libre);
      log_debug(logger, "Se liberó la CPU %d, ya que el proceso <%d> dejó de utilizar este recurso", cpu->id_cpu, pid);
      break;
    };
  };
  pthread_mutex_unlock(&mutex_cpus);
};