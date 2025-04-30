#ifndef MAIN_H_
#define MAIN_H_

#include "./utils/utils.h"
#include "common_kernel.h"

extern int conexion_cpu_dispatch;
extern int conexion_cpu_interrupt;
extern int conexion_io;
extern int conexion_memoria;

extern pthread_t hilo_cpu_dispatch;
extern pthread_t hilo_cpu_interrupt;
extern pthread_t hilo_io;

#endif /* MAIN_H_ */

