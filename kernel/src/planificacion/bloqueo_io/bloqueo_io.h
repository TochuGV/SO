#ifndef BLOQUEO_IO_H_
#define BLOQUEO_IO_H_

#include "pcb/pcb.h"
#include "logger/logger.h"
#include "common/common.h"
#include "planificacion/planificadores/corto_plazo/corto_plazo.h"
#include "planificacion/planificadores/mediano_plazo/mediano_plazo.h"
#include "utils/utils.h" // Revisar después
#include <commons/collections/queue.h>
#include "./conexion/io/io.h"

void manejar_respuesta_io(uint32_t);

#endif /* BLOQUEO_IO_H_ */