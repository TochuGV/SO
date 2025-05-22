#include "logger.h"
#include <stdio.h>

///void log_syscall(){};

void log_creacion_proceso(uint32_t pid){
  log_info(logger, "## (<%d>) Se crea el proceso - Estado: NEW", pid);
};

void log_cambio_estado(uint32_t pid, char* estado_actual, char* estado_siguiente){
  log_info(logger, "## (<%d>) Pasa del estado <%s> al estado <%s>", pid, estado_actual, estado_siguiente);
};

//void log_motivo_bloqueo(){};
//void log_fin_io(){};
//void log_desalojo_sjf_srt(){};

void log_fin_proceso(uint32_t pid){
  log_info(logger, "## (<%d>) - Finaliza el proceso", pid);
};

void log_metricas_estado(char* buffer){
  log_info(logger, "%s", buffer);
};