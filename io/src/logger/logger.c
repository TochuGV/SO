#include "logger.h"
#include <stdio.h>

void log_inicio_io(uint32_t pid, uint32_t tiempo_io){
  log_info(logger, "## PID: <%d> - Inicio de IO - Tiempo: <%d>", pid, tiempo_io);
};

void log_finalizacion_io(uint32_t pid){
  log_info(logger, "## PID: <%d> - Fin de IO", pid);
};