#include "estados.h"

void entrar_estado(t_pcb* pcb, int estado){
  //t_temporal* cronometro1 = temporal_create();
  pcb->me[estado]++;
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