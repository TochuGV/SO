#ifndef MAIN_H_
#define MAIN_H_

#include "./utils/utils.h"

t_list* leer_archivo_instrucciones(char* path);
int* ubicar_proceso_en_memoria(int tamanio_proceso, t_list* lista_instrucciones);

#endif /* MAIN_H_ */

