#include "pmcp.h"

t_pcb* elegir_proceso_mas_chico(t_queue* cola_new, t_list* lista_tamanios){
  if(queue_is_empty(cola_new)) return NULL;
  t_list* elementos = list_create();
  while(!queue_is_empty(cola_new)){
    list_add(elementos, queue_pop(cola_new));
  };

  t_pcb* pcb_elegido = NULL;
  uint32_t tamanio_minimo = UINT32_MAX;

  for(int i = 0; i < list_size(elementos); i++){
    t_pcb* pcb_actual = list_get(elementos, i);
    for (int j = 0; j < list_size(lista_tamanios); j++){
      t_informacion_largo_plazo* entry = list_get(lista_tamanios, j);
      if(entry->pid == pcb_actual->pid && entry->tamanio < tamanio_minimo){
        pcb_elegido = pcb_actual;
        tamanio_minimo = entry->tamanio;
      } ;
    };
  };
  for(int i = 0; i < list_size(elementos); i++){
    t_pcb* pcb = list_get(elementos, i);
    if(pcb != pcb_elegido) queue_push(cola_new, pcb);
  };
  list_destroy(elementos);
  return pcb_elegido;
};