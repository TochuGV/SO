#ifndef COMMON_IO_H_
#define COMMON_IO_H_

#include "./utils/utils.h"

int32_t handshake_io(char* nombre, int conexion_kernel);
void atender_interrupcion(int conexion_kernel);

#endif /* COMMON_IO_H_ */

