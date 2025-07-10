#include "sjf.h"

void reordenar_cola_ready_por_estimacion(t_queue* cola_ready) {
  if (queue_is_empty(cola_ready)) return;
  t_list* lista_ready = cola_ready->elements;
  list_sort(lista_ready, (void*) comparar_estimaciones);
}

bool comparar_estimaciones(t_pcb* pcb1, t_pcb* pcb2) {
  double estimacion1 = obtener_estimacion(pcb1->pid);
  double estimacion2 = obtener_estimacion(pcb2->pid);
  return estimacion1 < estimacion2;
}

// Decide qué algoritmo usar dinámicamente según la config
t_pcb* obtener_proximo_proceso_ready(t_queue* cola_ready) {
  if (strcmp(ALGORITMO_CORTO_PLAZO, "SJF") == 0 || strcmp(ALGORITMO_CORTO_PLAZO, "SRT") == 0) {
    reordenar_cola_ready_por_estimacion(cola_ready);
    return queue_pop(cola_ready);
  }
    //return seleccionar_proceso_con_menor_estimacion(cola_ready);
  else
    return queue_pop(cola_ready); // FIFO por defecto
}

double estimar_proxima_rafaga(uint32_t pid, double rafaga_real_anterior) {
  double estimacion_anterior = obtener_estimacion(pid);
  return ALFA * rafaga_real_anterior + (1 - ALFA) * estimacion_anterior;
}

// Inicializa la estimación de ráfaga para un nuevo proceso al valor por defecto.
void inicializar_estimacion_rafaga(uint32_t pid) {
   char* clave = string_itoa(pid);
   double* estimacion = malloc(sizeof(double));
   *estimacion = ESTIMACION_INICIAL;
   dictionary_put(diccionario_estimaciones, clave, estimacion);
   free(clave);
}

double obtener_estimacion(uint32_t pid) {
   char* clave = string_itoa(pid);
   double* valor = dictionary_get(diccionario_estimaciones, clave);
   double estimacion = valor ? *valor : ESTIMACION_INICIAL;
   free(clave);
   return estimacion;
}

// Esta función agarra la ráfaga real que usó el proceso y actualiza su estimación.
// Básicamente, aplicamos la fórmula de SJF: nueva = ALFA * real + (1 - ALFA) * anterior
void actualizar_estimacion(uint32_t pid, double estimacion_real) {
  // Convertimos el pid a string porque el diccionario usa claves tipo texto
  char* clave = string_itoa(pid);
  // Buscamos la estimación que teníamos guardada para este proceso
  double* estimacion_actual = dictionary_get(diccionario_estimaciones, clave);
  // Si no había nada cargado, usamos la estimación inicial que tenemos por config
  double anterior = estimacion_actual ? *estimacion_actual : ESTIMACION_INICIAL;
   // Reservamos espacio para la nueva estimación y aplicamos la fórmula mágica
  double* nueva = malloc(sizeof(double));
  *nueva = ALFA * estimacion_real + (1 - ALFA) * anterior;
  // Eliminamos la vieja estimación y guardamos la nueva. 
  dictionary_remove_and_destroy(diccionario_estimaciones, clave, free);
  dictionary_put(diccionario_estimaciones, clave, nueva);
  // Liberamos la clave porque la creamos con string_itoa
  //(y eso usa malloc --> funcion que pide al so un bloque de memoria)
  free(clave);
};

void destruir_diccionario_estimaciones() {
    dictionary_destroy_and_destroy_elements(diccionario_estimaciones, free);
};

void mover_proceso_a_exec_sjf(void){
  pthread_mutex_lock(&mutex_ready);
  if(queue_is_empty(cola_ready)){
    pthread_mutex_unlock(&mutex_ready);
    return;
  };

  t_cpu* cpu = seleccionar_cpu_disponible();
  if(!cpu){
    pthread_mutex_unlock(&mutex_ready);
    return;
  };

  reordenar_cola_ready_por_estimacion(cola_ready);
  t_pcb* pcb = queue_pop(cola_ready);
  pthread_mutex_unlock(&mutex_ready);

  log_info(logger, "Asignando proceso %d a CPU %d (SJF)", pcb->pid, cpu->id_cpu);
  asignar_proceso_a_cpu(cpu, pcb);
  enviar_a_cpu(pcb, cpu->socket_dispatch);
  cambiar_estado(pcb, ESTADO_READY, ESTADO_EXEC);
};