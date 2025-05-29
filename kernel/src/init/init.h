#ifndef INIT_H_
#define INIT_H_

#include "./utils/utils.h"
//#include "./common/common.h"

extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;
extern char* PUERTO_ESCUCHA_DISPATCH;
extern char* PUERTO_ESCUCHA_INTERRUPT;
extern char* PUERTO_ESCUCHA_IO;
extern char* ALGORITMO_CORTO_PLAZO;
extern char* ALGORITMO_INGRESO_A_READY;
extern char* ALFA;
extern char* ESTIMACION_INICIAL;
extern char* TIEMPO_SUSPENSION;
extern char* LOG_LEVEL;

extern t_list* lista_cpus;

extern pthread_t hilo_conexion_cpu_dispatch;
extern pthread_t hilo_conexion_cpu_interrupt;
extern pthread_t hilo_conexion_io;

void inicializar_kernel();
void extraer_datos_config();
//void crear_hilos();
//void unir_hilos();

#endif /* INIT_H_ */
