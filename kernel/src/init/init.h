#ifndef INIT_H_
#define INIT_H_

#include "common/common.h"
#include "cpu/cpu.h"
#include "conexion/cpu/dispatch/dispatch.h"
#include "conexion/cpu/interrupt/interrupt.h"
#include "conexion/io/io.h"
#include "conexion/handshake/entrante/entrante.h"
#include "planificacion/planificacion.h"
#include "utils/utils.h"

extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;
extern char* PUERTO_ESCUCHA_DISPATCH;
extern char* PUERTO_ESCUCHA_INTERRUPT;
extern char* PUERTO_ESCUCHA_IO;
extern char* ALGORITMO_CORTO_PLAZO;
extern char* ALGORITMO_INGRESO_A_READY;
extern double ALFA;
extern double ESTIMACION_INICIAL;
extern int32_t TIEMPO_SUSPENSION;

extern t_list* lista_pcbs;

extern t_dictionary* diccionario_dispositivos;
extern t_dictionary* diccionario_cronometros;
extern t_dictionary* diccionario_contextos_io;
extern t_dictionary* diccionario_estimaciones;

extern pthread_mutex_t mutex_pcbs;
extern pthread_mutex_t mutex_memoria;
extern pthread_mutex_t mutex_diccionario_cronometros;
extern pthread_mutex_t mutex_diccionario_contextos;
extern pthread_mutex_t mutex_diccionario_dispositivos;

extern pthread_t hilo_conexion_cpu_dispatch;
extern pthread_t hilo_conexion_cpu_interrupt;
extern pthread_t hilo_conexion_io;
extern pthread_t hilo_planificador_largo_plazo;
extern pthread_t hilo_planificador_corto_plazo;
extern pthread_t hilo_planificador_mediano_plazo;

void inicializar_kernel();
void crear_proceso_inicial(char*, uint32_t);
void iniciar_conexiones_constantes_entre_modulos();
void iniciar_planificadores();
void unir_hilos();
void finalizar_kernel();

#endif /* INIT_H_ */
