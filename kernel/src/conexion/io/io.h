#ifndef IO_H_
#define IO_H_

#include "./common/common.h"
#include "./conexion/handshake/entrante/entrante.h"

void* conectar_io(void*);
void* atender_io(void*);
void registrar_socket_io(char*, int);
void enviar_peticion_io(int, uint32_t, uint32_t);

#endif /* IO_H_ */