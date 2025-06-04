#include "planificacion.h"
#include <commons/temporal.h>

t_queue* cola_new;
t_queue* cola_ready;

pthread_mutex_t mutex_new = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_ready = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cpus = PTHREAD_MUTEX_INITIALIZER;

sem_t hay_procesos_en_new;

void iniciar_planificacion_largo_plazo(){
  cola_new = queue_create();
  sem_init(&hay_procesos_en_new, 0, 0);
  pthread_mutex_init(&mutex_new, NULL); 
};

void iniciar_planificacion_corto_plazo() {
  cola_ready = queue_create();
  pthread_mutex_init(&mutex_ready, NULL);
  pthread_mutex_init(&mutex_cpus, NULL);
};

void entrar_estado(t_pcb* pcb, int estado){
  //t_temporal* cronometro1 = temporal_create();
  pcb->me[estado]++;
};

void inicializar_proceso(t_pcb *pcb){
  queue_push(cola_new, pcb);
  entrar_estado(pcb, ESTADO_NEW);
  //log_info(logger, "## (<%d>) Se crea el proceso - Estado: NEW", pcb->pid);
  log_creacion_proceso(pcb->pid);
};

void cambiar_estado(t_pcb* pcb, t_estado actual, t_estado siguiente){
  entrar_estado(pcb, siguiente);
  //log_info(logger, "## (<%d>) Pasa del estado <%s> al estado <%s>", pcb->pid, obtener_nombre_estado(actual), obtener_nombre_estado(siguiente));
  log_cambio_estado(pcb->pid, obtener_nombre_estado(actual), obtener_nombre_estado(siguiente));
};

void finalizar_proceso(t_pcb* pcb){
  //cambiar_estado(pcb, ESTADO_EXEC, ESTADO_EXIT);
  //Revisar que de cualquier estado puede pasar a EXIT.
  //Revisar si tiene sentido los campos 'me' y 'mt' para EXIT.
  
  //log_info(logger, "## (<%d>) - Finaliza el proceso", pcb->pid);
  log_fin_proceso(pcb->pid);

  char* buffer = crear_cadena_metricas_estado(pcb);
  //log_info(logger, "%s", buffer);
  log_metricas_estado(buffer);
  free(buffer);
  destruir_pcb(pcb);
};

void mover_proceso_a_ready(char* archivo_pseudocodigo, int32_t tamanio_proceso) {
  pthread_mutex_lock(&mutex_new);
  if(queue_is_empty(cola_new)){
    pthread_mutex_unlock(&mutex_new);
    return;
  };

  t_pcb* pcb = queue_pop(cola_new);
  pthread_mutex_unlock(&mutex_new);
  if(enviar_proceso_a_memoria(archivo_pseudocodigo, tamanio_proceso, pcb->pid, conexion_memoria) == 0) {
    pthread_mutex_lock(&mutex_ready);
    queue_push(cola_ready, pcb); 
    pthread_mutex_unlock(&mutex_ready); 
    cambiar_estado(pcb, ESTADO_NEW, ESTADO_READY);
  } else {
    cambiar_estado(pcb, ESTADO_NEW, ESTADO_EXIT); //Por ahora queda así
    finalizar_proceso(pcb);
  };
};

void enviar_a_cpu(t_pcb* pcb, int socket_cpu_dispatch){
  int bytes = sizeof(uint32_t) * (2 + 2 * CANTIDAD_ESTADOS);
  void* stream = serializar_pcb(pcb, bytes);
  t_paquete* paquete = crear_paquete(PCB);
  agregar_a_paquete(paquete, stream, bytes);
  log_debug(logger, "Enviando PCB del Proceso <%d> al socket CPU Dispatch: %d", pcb->pid, socket_cpu_dispatch);
  enviar_paquete(paquete, socket_cpu_dispatch);
  log_info(logger, "PCB del Proceso <%d> enviado a CPU", pcb->pid);
  free(stream);
};

void mover_proceso_a_exec(){
  pthread_mutex_lock(&mutex_ready); 
  if (queue_is_empty(cola_ready)) { 
    log_info(logger, "No hay procesos en la cola READY"); 
    pthread_mutex_unlock(&mutex_ready); 
    return;
  };
  t_pcb* pcb_elegido = queue_pop(cola_ready);
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

void* planificador_ciclo_general(void* arg){
  while(1){
    mover_proceso_a_ready(NULL, 0);
    mover_proceso_a_exec();
    sleep(1);
  };
  return NULL;
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
      log_debug(logger, "Se liberó la CPU %d, ya que el proceso <%d> finalizó", cpu->id_cpu, pid);
      break;
    };
  };
  pthread_mutex_unlock(&mutex_cpus);
};