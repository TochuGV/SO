#ifndef COMMON_MEMORIA_H_
#define COMMON_MEMORIA_H_

#include "./utils/utils.h"

t_list* leer_archivo_instrucciones(char* path);
void* ubicar_proceso_en_memoria(int tamanio_proceso, t_list* lista_instrucciones);
void* recibir_path(int socket_cliente);

#endif /* COMMON_MEMORIA_H_ */