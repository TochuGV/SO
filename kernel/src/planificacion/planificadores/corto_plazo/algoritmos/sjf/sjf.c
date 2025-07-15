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

double estimar_proxima_rafaga(uint32_t pid, double rafaga_real_anterior) {
  double estimacion_anterior = obtener_estimacion(pid);
  return ALFA * rafaga_real_anterior + (1 - ALFA) * estimacion_anterior;
}

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

void actualizar_estimacion(uint32_t pid, double estimacion_real) {
  char* clave = string_itoa(pid);
  double* estimacion_actual = dictionary_get(diccionario_estimaciones, clave);
  double anterior = estimacion_actual ? *estimacion_actual : ESTIMACION_INICIAL;
  double* nueva = malloc(sizeof(double));
  *nueva = ALFA * estimacion_real + (1 - ALFA) * anterior;
  dictionary_remove_and_destroy(diccionario_estimaciones, clave, free);
  dictionary_put(diccionario_estimaciones, clave, nueva);
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
  asignar_y_enviar_a_cpu(pcb, cpu);
};