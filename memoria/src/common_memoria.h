#ifndef COMMON_MEMORIA_H_
#define COMMON_MEMORIA_H_

#include "./utils/utils.h"

extern int conexion_kernel;

t_list* leer_archivo_instrucciones(char* path);
void* ubicar_proceso_en_memoria(int tamanio_proceso, t_list* lista_instrucciones);
void* recibir_path(int socket_cliente);
void* atender_kernel(void* arg);
void* conectar_kernel(void* arg);

#endif /* COMMON_MEMORIA_H_ */