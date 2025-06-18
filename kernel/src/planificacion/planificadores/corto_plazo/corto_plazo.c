#include "corto_plazo.h"

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
  if (queue_is_empty(cola_ready)) { 
    log_info(logger, "No hay procesos en la cola READY"); 
    pthread_mutex_unlock(&mutex_ready); 
    return;
  };
  t_pcb* pcb_elegido = obtener_proximo_proceso_ready();

  if (strcmp(ALGORITMO_CORTO_PLAZO, "SJF") == 0) {
    log_info(logger, "SJF seleccionó PID <%d> con estimación %.2f",
             pcb_elegido->pid, pcb_elegido->estimacion_rafaga);
}

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
    
    pthread_mutex_lock(&mutex_ready);
    queue_push(cola_ready, pcb_elegido); //Estas 3 líneas son opcionales por si no se puede asignar una CPU
    pthread_mutex_unlock(&mutex_ready);
    
    return;
  };
  log_info(logger, "Asignando proceso %d a CPU %d", pcb_elegido->pid, cpu_seleccionada->id_cpu);
  enviar_a_cpu(pcb_elegido, cpu_seleccionada->socket_dispatch);
  cambiar_estado(pcb_elegido, ESTADO_READY, ESTADO_EXEC);
};

void enviar_a_cpu(t_pcb* pcb, int socket_cpu_dispatch){
  int bytes;
  void* stream = serializar_pcb_para_cpu(pcb, &bytes);
  t_paquete* paquete = crear_paquete(PCB);
  agregar_a_paquete(paquete, stream, bytes);
  //log_debug(logger, "Enviando PCB del Proceso <%d> al socket CPU Dispatch: %d", pcb->pid, socket_cpu_dispatch);
  enviar_paquete(paquete, socket_cpu_dispatch);
  //log_debug(logger, "PCB del Proceso <%d> enviado a CPU", pcb->pid);
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
      sem_post(&semaforo_cpu_libre);
      log_debug(logger, "Se liberó la CPU %d, ya que el proceso <%d> dejó de utilizar este recurso", cpu->id_cpu, pid);
      break;
    };
  };
  pthread_mutex_unlock(&mutex_cpus);
};

// Recorre la cola READY y selecciona el proceso con menor estimación de ráfaga
t_pcb* seleccionar_proceso_con_menor_estimacion(t_queue* cola_ready) {
    if (queue_is_empty(cola_ready))
        return NULL;  // Si la cola está vacía, no hay nada para planificar

    // Tomamos la lista interna de la queue para poder iterarla
    t_list* lista_ready = cola_ready->elements;

    // Suponemos que el primer PCB es el menor para iniciar la comparación
    t_pcb* pcb_menor = list_get(lista_ready, 0);

    // Recorremos el resto buscando el de menor estimación
    for (int i = 1; i < list_size(lista_ready); i++) {
        t_pcb* pcb_actual = list_get(lista_ready, i);
        if (pcb_actual->estimacion_rafaga < pcb_menor->estimacion_rafaga) {
            pcb_menor = pcb_actual;
        }
    }

    // Lo eliminamos de la cola (es planificación sin desalojo → remueve de READY)
    list_remove_element(lista_ready, pcb_menor);
    return pcb_menor;
}

// Decide qué algoritmo usar dinámicamente según la config
t_pcb* obtener_proximo_proceso_ready() {
    if (strcmp(ALGORITMO_CORTO_PLAZO, "SJF") == 0)
        return seleccionar_proceso_con_menor_estimacion(cola_ready);
    else
        return queue_pop(cola_ready);  // FIFO por defecto
}
