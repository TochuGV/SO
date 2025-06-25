#ifndef COMMON_H_
#define COMMON_H_

#include "./utils/utils.h"
#include "init/init.h"
#include "pcb/pcb.h"
#include <stdint.h>
#include <commons/collections/queue.h>
#include <commons/temporal.h>
#include "conexion/handshake/saliente/saliente.h"
#include "./planificacion/algoritmos/srt/srt.h"

typedef struct {
  int32_t id_cpu;
  int socket_dispatch;
  int socket_interrupt;
  bool disponible;
  t_pcb* proceso_en_ejecucion;
} t_cpu;

typedef struct {
  char* dispositivo_actual;
  uint32_t duracion_io;
} t_contexto_io;

typedef struct {
  t_queue* cola_bloqueados;
  bool ocupado;
  int socket;
} t_dispositivo_io;

typedef struct {
  t_temporal* cronometros_estado[CANTIDAD_ESTADOS];
} t_temporizadores_estado;

extern char* NOMBRES_SYSCALLS[4];
extern char* NOMBRES_DISPOSITIVOS_IO[5];

uint32_t enviar_proceso_a_memoria(char*, uint32_t, uint32_t);
void esperar_enter_para_planificar();
char* crear_cadena_metricas_estado(t_pcb*);

char* token_io_to_string(int32_t);
t_pcb* obtener_pcb_por_pid(uint32_t);
void destruir_contexto_io(void*);
void encolar_proceso_en_ready(t_pcb*);

#endif /* COMMON_H_ */