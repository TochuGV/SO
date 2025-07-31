#include "srt.h"

bool es_SRT() {
  return strcmp(ALGORITMO_CORTO_PLAZO, "SRT") == 0;
}

double tiempo_restante_exec(t_pcb* pcb) {
  char* clave = string_itoa(pcb->pid);
  t_temporizadores_estado* tiempos = dictionary_get(diccionario_cronometros, clave);
  free(clave);

  if (!tiempos || !tiempos->cronometros_estado[ESTADO_EXEC]) {
    return obtener_estimacion(pcb->pid);
  };

  double estimacion_total = obtener_estimacion(pcb->pid);
  double ejecutado = temporal_gettime(tiempos->cronometros_estado[ESTADO_EXEC]);
  //log_debug(logger, "EJECUTADO: %.2f", ejecutado);
  double restante = estimacion_total - ejecutado;
  //log_debug(logger, "RESTANTE: %.2f", restante);
  return restante > 0 ? restante : 0.0;
}

t_pcb* obtener_proceso_en_exec_con_mayor_estimacion(int cantidad_cpus) {
  t_pcb* peor_pcb = NULL;
  double mayor_tiempo_restante = -1.0;
  
  for(int i = 0; i < cantidad_cpus; i++){
    pthread_mutex_lock(&mutex_cpus);
    t_cpu* cpu = list_get(lista_cpus, i);
    pthread_mutex_unlock(&mutex_cpus);

    if(cpu->disponible)
      return NULL;

    if(!cpu->disponible && cpu->proceso_en_ejecucion){
      t_pcb* ejecutando = cpu->proceso_en_ejecucion;
      double tiempo_restante = tiempo_restante_exec(ejecutando);

      if(tiempo_restante > mayor_tiempo_restante){
        peor_pcb = ejecutando;
        mayor_tiempo_restante = tiempo_restante;
      };
    };
  };
  return peor_pcb;
};

void desalojar_cpu(t_pcb* pcb) {
  if (!es_SRT()) return;

  double estimacion_nuevo = obtener_estimacion(pcb->pid);
  int cantidad_cpus = obtener_cantidad_cpus();

  t_pcb* proceso_en_exec = obtener_proceso_en_exec_con_mayor_estimacion(cantidad_cpus);
  if (proceso_en_exec == NULL) return;

  double tiempo_restante = tiempo_restante_exec(proceso_en_exec);
  log_debug(logger, "SRT - Evaluando desalojo: PID nuevo %d (%.2f) vs PID en EXEC %d (%.2f)", pcb->pid, estimacion_nuevo, proceso_en_exec->pid, tiempo_restante);
  //double estimacion_exec = obtener_estimacion(proceso_en_exec->pid);
  //log_debug(logger, "SRT - Evaluando desalojo: PID nuevo %d (%.2f) vs PID en EXEC %d (%.2f)", pcb->pid, estimacion_nuevo, proceso_en_exec->pid, estimacion_exec);

  if(estimacion_nuevo < tiempo_restante){
    t_cpu* cpu = obtener_cpu_que_ejecuta(proceso_en_exec->pid);
    if(!cpu) return;
    send(cpu->socket_interrupt, &(proceso_en_exec->pid), sizeof(uint32_t), 0);
  } else {
    log_debug(logger, "SRT - No se desaloja: %.2f >= %.2f", estimacion_nuevo, tiempo_restante);
    //log_debug(logger, "SRT - No se desaloja: %.2f >= %.2f", estimacion_nuevo, estimacion_exec);
  };
};

void mover_proceso_a_exec_srt(void){
  pthread_mutex_lock(&mutex_ready);
  if(queue_is_empty(cola_ready)){
    pthread_mutex_unlock(&mutex_ready);
    return;
  };

  reordenar_cola_ready_por_estimacion(cola_ready);
  t_pcb* pcb = queue_peek(cola_ready);
  pthread_mutex_unlock(&mutex_ready);

  t_cpu* cpu = seleccionar_cpu_disponible();
  if(cpu != NULL){
    pthread_mutex_lock(&mutex_ready);
    pcb = queue_pop(cola_ready);
    pthread_mutex_unlock(&mutex_ready);
    asignar_y_enviar_a_cpu(pcb, cpu);
    return;
  };
  return;
};