#ifndef INIT_H_
#define INIT_H_

#include "./utils/utils.h"
#include "./common/common.h"

extern char* IP_KERNEL;
extern char* PUERTO_KERNEL;

void inicializar_io();
void extraer_datos_config();

#endif /* INIT_H_ */
