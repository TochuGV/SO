#include "sjf.h"

//t_dictionary* diccionario_estimaciones; //para la estimacion de rafaga

/*
* sjf.c - Módulo de planificación SJF sin desalojo
* Calcula estimaciones de ráfaga y selecciona el próximo proceso READY.
* Usa un diccionario <pid, estimación> para trackear historiales.
*/
/*
// Recorre la cola READY y selecciona el proceso con menor estimación de ráfaga
t_pcb* seleccionar_proceso_con_menor_estimacion(t_queue* cola_ready) {
   // Si la cola está vacía, no hay proceso a elegir.
  if (queue_is_empty(cola_ready)) return NULL;

   // Accedemos directamente a la lista interna de la cola para iterarla.
  t_list* lista_ready = cola_ready->elements;

   // Suponemos que el primer proceso es el de menor estimación.
  t_pcb* pcb_menor = list_get(lista_ready, 0);
  double menor_estimacion = obtener_estimacion(pcb_menor->pid);

   // Iteramos el resto de los procesos para encontrar el de menor estimación.
  for (int i = 1; i < list_size(lista_ready); i++) {
    t_pcb* pcb_actual = list_get(lista_ready, i);
    double estimacion_actual = obtener_estimacion(pcb_actual->pid);

    if (estimacion_actual < menor_estimacion) {
      pcb_menor = pcb_actual;
      menor_estimacion = estimacion_actual;
    }
  }

   // Eliminamos el proceso elegido de la cola (porque es planificación sin desalojo).
  list_remove_element(lista_ready, pcb_menor);
   
   // Retornamos el proceso con menor estimación.
  return pcb_menor;
}
*/

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
}

