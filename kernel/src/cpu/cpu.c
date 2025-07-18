#include "cpu.h"

t_list* lista_cpus;
pthread_mutex_t mutex_cpus = PTHREAD_MUTEX_INITIALIZER;
sem_t semaforo_cpu_libre;

void inicializar_estructura_cpus(void){
  lista_cpus = list_create();
  pthread_mutex_init(&mutex_cpus, NULL);
  sem_init(&semaforo_cpu_libre, 0, 0);
};

t_cpu* obtener_cpu_por_id(int id_cpu){
  bool misma_cpu(void* elem){
    return ((t_cpu*)elem)->id_cpu == id_cpu;
  };
  pthread_mutex_lock(&mutex_cpus);
  t_cpu* cpu = list_find(lista_cpus, misma_cpu);
  pthread_mutex_unlock(&mutex_cpus);
  return cpu;
};

void registrar_cpu_si_no_existe(int id_cpu){
  if(obtener_cpu_por_id(id_cpu) != NULL) return;
  t_cpu* nueva = malloc(sizeof(t_cpu));
  nueva->id_cpu = id_cpu;
  nueva->socket_dispatch = -1;
  nueva->socket_interrupt = -1;
  nueva->disponible = true;
  nueva->proceso_en_ejecucion = NULL;
  pthread_mutex_lock(&mutex_cpus);
  list_add(lista_cpus, nueva);
  pthread_mutex_unlock(&mutex_cpus);
};

bool cpu_esta_completa(t_cpu* cpu){
  return cpu->socket_dispatch != -1 && cpu->socket_interrupt != -1;
};

void asignar_proceso_a_cpu(t_cpu* cpu, t_pcb* pcb){
  cpu->disponible = false;
  cpu->proceso_en_ejecucion = pcb;
};

void liberar_cpu(t_cpu* cpu){
  cpu->disponible = true;
  cpu->proceso_en_ejecucion = NULL;
};

void liberar_cpu_por_pid(uint32_t pid){
  pthread_mutex_lock(&mutex_cpus);
  for(int i = 0; i < list_size(lista_cpus); i++){
    t_cpu* cpu = list_get(lista_cpus, i);
    if(!cpu->disponible && cpu->proceso_en_ejecucion && cpu->proceso_en_ejecucion->pid == pid){
      liberar_cpu(cpu);
      break;
    };
  };
  pthread_mutex_unlock(&mutex_cpus);
  //int valor_antes;
  //sem_getvalue(&semaforo_cpu_libre, &valor_antes);
  //log_debug(logger, "[liberar_cpu_por_pid - Antes de sem_post] Semáforo CPU libre: %d", valor_antes);
  
  sem_post(&semaforo_cpu_libre);

  //int valor_despues;
  //sem_getvalue(&semaforo_cpu_libre, &valor_despues);
  //log_debug(logger, "[liberar_cpu_por_pid - Después de sem_post] Semáforo CPU libre: %d", valor_despues);
};

t_cpu* seleccionar_cpu_disponible(void){
  pthread_mutex_lock(&mutex_cpus);
  for(int i = 0; i < list_size(lista_cpus); i++){
    t_cpu* cpu = list_get(lista_cpus, i);
    if(cpu->disponible){
      cpu->disponible = false;
      pthread_mutex_unlock(&mutex_cpus);
      return cpu;
    };
  };
  pthread_mutex_unlock(&mutex_cpus);
  return NULL;
};

t_cpu* obtener_cpu_que_ejecuta(uint32_t pid){
  pthread_mutex_lock(&mutex_cpus);
  for(int i = 0; i < list_size(lista_cpus); i++){
    t_cpu* cpu = list_get(lista_cpus, i);
    if(!cpu->disponible && cpu->proceso_en_ejecucion && cpu->proceso_en_ejecucion->pid == pid){
      pthread_mutex_unlock(&mutex_cpus);
      return cpu;
    };
  };
  pthread_mutex_unlock(&mutex_cpus);
  return NULL;
};

/*
void marcar_cpu_como_disponible(t_cpu* cpu){
  cpu->disponible = true;
  cpu->proceso_en_ejecucion = NULL;
  sem_post(&semaforo_cpu_libre);
};
*/

int obtener_cantidad_cpus(){
  pthread_mutex_lock(&mutex_cpus);
  int cantidad = list_size(lista_cpus);
  pthread_mutex_unlock(&mutex_cpus);
  return cantidad;
};