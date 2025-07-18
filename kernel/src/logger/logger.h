#ifndef LOGGER_H_
#define LOGGER_H_

#include "utils/utils.h"
#include "common/common.h"
#include <stdio.h>

void log_syscall_recibida(uint32_t, tipo_syscall);
void log_creacion_proceso(uint32_t);
void log_cambio_estado(uint32_t, t_estado, t_estado);
void log_motivo_bloqueo(uint32_t, char*);
void log_fin_io(uint32_t, t_estado);
void log_desalojo_sjf_srt(uint32_t);
void log_fin_proceso(uint32_t);
void log_metricas_estado(char*);

#endif