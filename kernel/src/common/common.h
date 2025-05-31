#ifndef COMMON_H_
#define COMMON_H_

#include "./utils/utils.h"
#include "init/init.h"
#include "pcb/pcb.h"
#include <stdint.h>

extern int conexion_cpu_dispatch;
extern int conexion_cpu_interrupt;
extern int conexion_io;
extern int conexion_memoria;

//t_list* lista_cpus; // Lista global de CPUs disponibles
typedef struct {
  int32_t id_cpu;
  int socket_dispatch;
  int socket_interrupt;
  bool disponible;
} t_cpu;

void* conectar_cpu_dispatch(void*);
void* atender_cpu_dispatch(void*);
void* conectar_cpu_interrupt(void*);
void* atender_cpu_interrupt(void*);
void* conectar_io(void*);
void* atender_io(void*);
int recibir_handshake_kernel(int);
void* atender_cliente(void*);
uint32_t enviar_proceso_a_memoria(char*, uint32_t, uint32_t, int);
int32_t handshake_kernel(int);
char* crear_cadena_metricas_estado(t_pcb*);

extern char* NOMBRES_SYSCALLS[4];
extern char* NOMBRES_DISPOSITIVOS_IO[5];

char* token_io_to_string(int32_t);
t_pcb* obtener_pcb_por_pid(uint32_t);

#endif /* COMMON_H_ */