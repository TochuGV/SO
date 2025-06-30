#include "corto_plazo.h"
#include "planificacion/algoritmos/sjf/sjf.h"

t_queue* cola_ready;
sem_t semaforo_ready;
pthread_mutex_t mutex_ready = PTHREAD_MUTEX_INITIALIZER;

void iniciar_planificacion_corto_plazo() {
  cola_ready = queue_create();
  sem_init(&semaforo_ready, 0, 0);
  pthread_mutex_init(&mutex_ready, NULL);
};

void mover_proceso_a_exec(){
  pthread_mutex_lock(&mutex_ready);

  /*
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
  */

  if (queue_is_empty(cola_ready)) { 
    log_info(logger, "No hay procesos en la cola READY"); 
    pthread_mutex_unlock(&mutex_ready); 
    return;
  };
  
  t_pcb* pcb_elegido = obtener_proximo_proceso_ready(cola_ready);
  if (pcb_elegido == NULL) {
    log_warning(logger, "No se pudo obtener un proceso válido para ejecutar");
    pthread_mutex_unlock(&mutex_ready);
    return;
  };
  log_debug(logger, "Proceso elegido PID <%d> para ejecutar", pcb_elegido->pid);

  pthread_mutex_unlock(&mutex_ready);
  t_cpu* cpu_seleccionada = seleccionar_cpu_disponible();
  if(cpu_seleccionada == NULL){
    log_info(logger, "No hay CPUs disponibles. Esperando...");
    encolar_proceso_en_ready(pcb_elegido);
    return;
  };

  log_info(logger, "Asignando proceso %d a CPU %d", pcb_elegido->pid, cpu_seleccionada->id_cpu);
  asignar_proceso_a_cpu(cpu_seleccionada, pcb_elegido);
  enviar_a_cpu(pcb_elegido, cpu_seleccionada->socket_dispatch);
  cambiar_estado(pcb_elegido, ESTADO_READY, ESTADO_EXEC);
};

void enviar_a_cpu(t_pcb* pcb, int socket_cpu_dispatch){
  int bytes;
  void* stream = serializar_pcb(pcb, &bytes);
  t_paquete* paquete = crear_paquete(PCB);
  agregar_a_paquete(paquete, stream, bytes);
  enviar_paquete(paquete, socket_cpu_dispatch);
  free(stream);
};