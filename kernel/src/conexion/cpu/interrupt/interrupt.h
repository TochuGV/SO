#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#include "common/common.h"
#include "conexion/conexion.h"
#include "conexion/handshake/entrante/entrante.h"
#include "conexion/cpu/desconexion/desconexion.h"

void* conectar_cpu_interrupt(void*);
void* atender_cpu_interrupt(void*);

#endif /* INTERRUPT_H_ */