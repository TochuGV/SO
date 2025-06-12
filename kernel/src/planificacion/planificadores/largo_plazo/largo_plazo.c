#include "largo_plazo.h"

t_queue* cola_new;
pthread_mutex_t mutex_new = PTHREAD_MUTEX_INITIALIZER;
sem_t hay_procesos_en_new;

void iniciar_planificacion_largo_plazo(){
  cola_new = queue_create();
  sem_init(&hay_procesos_en_new, 0, 0);
  pthread_mutex_init(&mutex_new, NULL); 
};

void inicializar_proceso(t_pcb *pcb){
  queue_push(cola_new, pcb);
  list_add(lista_pcbs, pcb);
  entrar_estado(pcb, ESTADO_NEW);
  log_creacion_proceso(pcb->pid);
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

/*
//Funcion que intente ingresar procesos a Ready si hay espacio
void intentar_ingresar_procesos_a_ready() {
    while (!list_is_empty(lista_pcbs_new)) {
        if (hay_espacio_en_ready()) {  
            t_pcb* pcb_nuevo = list_remove(lista_pcbs_new, 0);
            cambiar_estado(pcb_nuevo, ESTADO_NEW, ESTADO_READY);
            queue_push(cola_ready, pcb_nuevo); // Enviar el proceso a READY
            log_info(logger, "Proceso <%d> movido a READY", pcb_nuevo->pid);
        } else {
            log_debug(logger, "No hay espacio disponible en READY. Procesos en NEW esperando...");
            break;
        }
    }
}
*/