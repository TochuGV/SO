#ifndef DISPATCH_H_
#define DISPATCH_H_

#include "./common/common.h"
#include "./conexion/handshake/entrante/entrante.h"
#include "./syscalls/syscalls.h"
#include "./pcb/pcb.h"

void* conectar_cpu_dispatch(void*);
void* atender_cpu_dispatch(void*);

#endif /* DISPATCH_H_ */