#ifndef IO_H_
#define IO_H_

#include "./common/common.h"
#include "./conexion/handshake/entrante/entrante.h"

void* conectar_io(void*);
void* atender_io(void*);

#endif /* IO_H_ */