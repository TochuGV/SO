#ifndef CPU_H_
#define CPU_H_

#include "utils/utils.h"
#include "pcb/pcb.h"
#include <semaphore.h>

typedef struct {
  int32_t id_cpu;
  int socket_dispatch;
  int socket_interrupt;
  bool disponible;
  t_pcb* proceso_en_ejecucion;
} t_cpu;

extern t_list* lista_cpus;
extern pthread_mutex_t mutex_cpus;
extern sem_t semaforo_cpu_libre;

void inicializar_estructura_cpus(void);
t_cpu* obtener_cpu_por_id(int id_cpu);
void registrar_cpu_si_no_existe(int id_cpu);
bool cpu_esta_completa(t_cpu* cpu);
void asignar_proceso_a_cpu(t_cpu* cpu, t_pcb* pcb);
void liberar_cpu(t_cpu* cpu);
void liberar_cpu_por_pid(uint32_t pid);
t_cpu* seleccionar_cpu_disponible(void);
t_cpu* obtener_cpu_que_ejecuta(uint32_t pid);
int obtener_cantidad_cpus();

#endif /* CPU_H */