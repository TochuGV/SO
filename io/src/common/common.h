#ifndef COMMON_H_
#define COMMON_H_

#include "./utils/utils.h"
#include "logger/logger.h"
#include <ctype.h>

int32_t handshake_io(char* nombre, int conexion_kernel);
void atender_interrupcion(int conexion_kernel);
void convertir_primera_letra_en_mayuscula(char*);

#endif /* COMMON_H_ */

