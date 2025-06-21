#include "srt.h"

/* Funciona igual que el anterior con la variante que, al ingresar un proceso en la cola de Ready y no haber CPUs libres, se debe evaluar si dicho proceso 
tiene una rafaga más corta que los que se encuentran en ejecución. En caso de ser así, se debe informar al CPU que posea al Proceso con el tiempo restante 
más alto que debe desalojar al mismo para que pueda ser planificado el nuevo.*/

bool es_SRT() {
  return strcmp(ALGORITMO_CORTO_PLAZO, "SRT") == 0;
}

// Calcula el tiempo restante de ejecución estimado para un proceso en EXEC
double tiempo_restante_exec(t_pcb* pcb) {

    char* clave = string_itoa(pcb->pid);
    t_temporizadores_estado* tiempos = dictionary_get(diccionario_cronometros, clave);// Obtenemos el registro de cronómetros asociados a ese proceso
    free(clave);

    // Validamos que exista un cronómetro para el estado EXEC, sino no podemos calcular el tiempo ejecutado
    if (!tiempos || !tiempos->cronometros_estado[ESTADO_EXEC]) {
        log_warning(logger, "No se pudo calcular el tiempo restante del proceso <%d>", pcb->pid);
        // Si no hay registro, devolvemos la estimación completa (asumimos que no ejecutó nada aún)
        return obtener_estimacion(pcb->pid);
    }

    double estimacion_total = obtener_estimacion(pcb->pid);  // Obtenemos la estimación total actual de ráfaga para este proceso
    double ejecutado = temporal_gettime(tiempos->cronometros_estado[ESTADO_EXEC]) / 1000.0;// Obtenemos cuánto tiempo lleva ejecutando este proceso (en milisegundos), lo pasamos a segundos
    double restante = estimacion_total - ejecutado;// Calculamos cuánto tiempo le queda por ejecutar (estimación - lo que ya ejecutó)
    return restante > 0 ? restante : 0.0;// Si el resultado dio negativo por algún motivodevolvemos 0 como mínimo
}


//fijarse el puntero proceso_en_ejecucion --> seteado para poder saber que proceso se está ejecutando. 
t_pcb* obtener_proceso_en_exec_con_mayor_estimacion(int cant_cpus) {
    t_pcb* peor_pcb = NULL;
    double mayor_tiempo_restante = -1.0;
    

    //pthread_mutex_lock(&mutex_cpus);
    log_warning(logger, "Cantidad CPUs: %d", cant_cpus);
    for (int i = 0; i < cant_cpus; i++) {
        pthread_mutex_lock(&mutex_cpus);
        t_cpu* cpu = list_get(lista_cpus, i);
        pthread_mutex_unlock(&mutex_cpus);
        if (!cpu->disponible && cpu->proceso_en_ejecucion) {
            t_pcb* ejecutando = cpu->proceso_en_ejecucion;
            log_warning(logger, "PID EJECUTANDO: %d", ejecutando->pid);
            double tiempo_restante = tiempo_restante_exec(ejecutando);

            if (tiempo_restante > mayor_tiempo_restante) {
                peor_pcb = ejecutando;
                log_warning(logger, "PID con mayor tiempo: %d", peor_pcb->pid);
                mayor_tiempo_restante = tiempo_restante;
            }
        }
    }
    //pthread_mutex_unlock(&mutex_cpus);

    return peor_pcb;
};

void desalojar_cpu(void) {
  if (!es_SRT()) return;

  //t_pcb* proceso_en_exec = obtener_proceso_en_exec_con_mayor_tiempo_restante();
  /*
  if (proceso_en_exec == NULL) {
    // No hay nadie ejecutando, no hay nada que interrumpir
    return;
  }
  */
  //double estimacion_nuevo = obtener_estimacion(nuevo_proceso_ready->pid);
  //double restante_exec = tiempo_restante_exec(proceso_en_exec);

  //log_info(logger, "SRT: El proceso <%d> interrumpe al <%d> (%.2f < %.2f)", nuevo_proceso_ready->pid, proceso_en_exec->pid, estimacion_nuevo, restante_exec);

  // Buscar la CPU que tiene ejecutando a este proceso y mandar interrupción
  pthread_mutex_lock(&mutex_cpus);
  int cant_cpus = list_size(lista_cpus);
  pthread_mutex_unlock(&mutex_cpus);

  t_pcb* proceso_en_exec = obtener_proceso_en_exec_con_mayor_estimacion(cant_cpus);
  if (proceso_en_exec == NULL) {
    // No hay nadie ejecutando, no hay nada que interrumpir
    //pthread_mutex_unlock(&mutex_cpus);
    return;
  }
  for (int i = 0; i < cant_cpus; i++) {
    pthread_mutex_lock(&mutex_cpus);
    t_cpu* cpu = list_get(lista_cpus, i);
    pthread_mutex_unlock(&mutex_cpus);
    if(!cpu->disponible && cpu->proceso_en_ejecucion && cpu->proceso_en_ejecucion->pid == proceso_en_exec->pid){
      int socket_interrupt = cpu->socket_interrupt;
      send(socket_interrupt, &(proceso_en_exec->pid), sizeof(uint32_t), 0);
      break;
    }
  }
  //pthread_mutex_unlock(&mutex_cpus);
}

