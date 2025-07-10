#include "mediano_plazo.h"

t_queue* cola_susp_blocked;
t_queue* cola_susp_ready;

void iniciar_planificacion_mediano_plazo(){
  cola_susp_blocked = queue_create();
  cola_susp_ready = queue_create();
};

void suspender_proceso(t_pcb* pcb){
  char* clave_pid = string_itoa(pcb->pid);
  t_temporizadores_estado* cronometros_pid = dictionary_get(diccionario_cronometros, clave_pid);
  if(cronometros_pid && cronometros_pid->cronometros_estado[ESTADO_BLOCKED]){
    if(cronometros_pid->cronometros_estado[ESTADO_BLOCKED] > TIEMPO_SUSPENSION){
      cambiar_estado(pcb, ESTADO_BLOCKED, ESTADO_SUSP_BLOCKED);
    };
  };
};