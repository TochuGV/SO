#include "estados.h"

void entrar_estado(t_pcb* pcb, int estado){
  pcb->me[estado]++;
  char* clave_pid = string_itoa(pcb->pid);
  t_temporizadores_estado* cronometros_pid = dictionary_get(diccionario_cronometros, clave_pid);
  if(!cronometros_pid){
    cronometros_pid = malloc(sizeof(t_temporizadores_estado));
    for(int i = 0; i < CANTIDAD_ESTADOS; i++){
      cronometros_pid->cronometros_estado[i] = NULL;
    } 
    dictionary_put(diccionario_cronometros, clave_pid, cronometros_pid);
  };
  if(cronometros_pid->cronometros_estado[estado]){
    temporal_destroy(cronometros_pid->cronometros_estado[estado]);
  };
  cronometros_pid->cronometros_estado[estado] = temporal_create();
  free(clave_pid);
};

void cambiar_estado(t_pcb* pcb, t_estado actual, t_estado siguiente){
  char* clave_pid = string_itoa(pcb->pid);
  t_temporizadores_estado* cronometros_pid = dictionary_get(diccionario_cronometros, clave_pid);
  if(cronometros_pid && cronometros_pid->cronometros_estado[actual]){ 
    uint32_t tiempo = temporal_gettime(cronometros_pid->cronometros_estado[actual]);
    pcb->mt[actual] += tiempo;
    temporal_destroy(cronometros_pid->cronometros_estado[actual]);
    cronometros_pid->cronometros_estado[actual] = NULL;
  };
  free(clave_pid);
  entrar_estado(pcb, siguiente);
  log_cambio_estado(pcb->pid, obtener_nombre_estado(actual), obtener_nombre_estado(siguiente));
};