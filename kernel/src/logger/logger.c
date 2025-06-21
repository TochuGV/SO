#include "logger.h"
#include <stdio.h>

void log_syscall_recibida(uint32_t pid, tipo_syscall syscall){
  log_info(logger, "## (<%d>) - Solicitó syscall: <%s>", pid, NOMBRES_SYSCALLS[syscall]);
};

void log_creacion_proceso(uint32_t pid){
  log_info(logger, "## (<%d>) Se crea el proceso - Estado: NEW", pid);
};

void log_cambio_estado(uint32_t pid, char* estado_actual, char* estado_siguiente){
  log_info(logger, "## (<%d>) Pasa del estado <%s> al estado <%s>", pid, estado_actual, estado_siguiente);
};

void log_motivo_bloqueo(uint32_t pid, nombre_dispositivo_io dispositivo){
  log_info(logger, "## (<%d>) - Bloqueado por IO: <%s>", pid, NOMBRES_DISPOSITIVOS_IO[dispositivo]);
};

void log_fin_io(uint32_t pid){
  log_info(logger, "## (<%d>) finalizó IO y pasa a READY", pid);
};

void log_desalojo_sjf_srt(uint32_t pid){
  log_info(logger, "## (<%d>) - Desalojado por algoritmo SJF/SRT", pid);
};

void log_fin_proceso(uint32_t pid){
  log_info(logger, "## (<%d>) - Finaliza el proceso", pid);
};

void log_metricas_estado(char* buffer){
  log_info(logger, "%s", buffer);
};