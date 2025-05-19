#include "planificacion.h"
#include <commons/temporal.h>

t_queue* cola_new;
t_queue* cola_ready;

//pthread_mutex_t mutex_new = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t mutex_ready = PTHREAD_MUTEX_INITIALIZER;
//sem_t hay_procesos_en_new;

void iniciar_planificacion_largo_plazo(void* args){
  cola_new = queue_create();
};

/*
void iniciar_planificacion_corto_plazo() {
    cola_ready = queue_create();
    pthread_mutex_init(&mutex_ready, NULL);
}
*/

void entrar_estado(t_pcb* pcb, int estado){
  //t_temporal* cronometro1 = temporal_create();
  pcb->me[estado]++;
};

//void salir_estado(t_pcb* pcb, int estado){
//}
/* void entrar_estado(t_pcb* pcb, int estado) {
    t_temporal* cronometro = temporal_create();
    pcb->me[estado]++;
    pcb->tiempo_estado_actual = temporal_get_time(cronometro); // Obtiene el tiempo en milisegundos
    temporal_destroy(cronometro); // Libera la estructura para evitar leaks
}
*/
void inicializar_proceso(t_pcb *pcb){
  queue_push(cola_new, pcb);
  entrar_estado(pcb, ESTADO_NEW);
  log_info(logger, "## (<%d>) Se crea el proceso - Estado: NEW", pcb->pid);
};

void cambiar_estado(t_pcb* pcb, t_estado actual, t_estado siguiente){
  log_info(logger, "## (<%d>) Pasa del estado <%s> al estado <%s>", pcb->pid, obtener_nombre_estado(actual), obtener_nombre_estado(siguiente));
  pcb->me[siguiente]++;
};

void finalizar_proceso(t_pcb* pcb){
  log_info(logger, "## (<%d>) - Finaliza el proceso", pcb->pid);
  
  //Métricas de estado...
  
  /*
  log_info(logger, "## (<%d>) - Métricas de estado: ", pcb->pid);
  for(int i = 0; i < CANTIDAD_ESTADOS; i++){
    log_info(logger, "%s (%d) (TIME)", obtener_nombre_estado(i), pcb->me[i]);
    if(i < CANTIDAD_ESTADOS - 1) log_info(logger, ", ");
  };
  */

  char* buffer = string_from_format("## (<%d>) - Métricas de estado: ", pcb->pid);
  for(int i = 0; i < CANTIDAD_ESTADOS; i++){
    char* aux = string_from_format("%s (%d) (%d)", obtener_nombre_estado(i), pcb->me[i], pcb->mt[i]);
    string_append(&buffer, aux);
    if(i < CANTIDAD_ESTADOS - 1) string_append(&buffer, ", ");
    free(aux);
  };

  log_info(logger, "%s", buffer);
  free(buffer);

  destruir_pcb(pcb);
};


//                                                                       COLA DE READY
/*
// Función que mueve un proceso de NEW a READY para que pueda ser planificado en la CPU
void mover_proceso_a_ready(t_pcb* pcb) {
    pthread_mutex_lock(&mutex_ready); // Bloqueamos el acceso a la cola READY para evitar condiciones de carrera
    queue_push(cola_ready, pcb); // Agregamos el proceso a la cola de READY, donde esperará a ser seleccionado para ejecución
    pthread_mutex_unlock(&mutex_ready); // Liberamos el acceso a la cola READY una vez que la operación se ha completado
    log_info(logger, "## (<%d>) Proceso movido a READY", pcb->pid); // Registramos el cambio de estado en los logs para seguimiento y depuración
}

// Creamos función para obtener el proceso siguiente
t_pcb* obtener_siguiente_proceso() {
    pthread_mutex_lock(&mutex_ready); //Bloquea el acceso a la cola READY para evitar condiciones de carrera

    if (queue_is_empty(cola_ready)) { //Verifica si la cola READY está vacía antes de intentar extraer un proceso
        pthread_mutex_unlock(&mutex_ready); //Si está vacía, libera el mutex antes de salir
        return NULL; //Retorna NULL indicando que no hay procesos disponibles en READY
    }
    t_pcb* pcb = queue_pop(cola_ready); //Extrae el primer proceso en la cola READY (FIFO por defecto)
    pthread_mutex_unlock(&mutex_ready); //Libera el acceso a la cola READY para permitir futuras operaciones
    log_info(logger, "## (<%d>) Proceso seleccionado para EXEC", pcb->pid); //Registra en los logs qué proceso ha sido seleccionado para ejecución
    return pcb; //Devuelve el proceso que se ejecutará en la CPU
}
*/

