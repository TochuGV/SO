#include "init.h"

char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA_DISPATCH;
char* PUERTO_ESCUCHA_INTERRUPT;
char* PUERTO_ESCUCHA_IO;
char* ALGORITMO_CORTO_PLAZO;
char* ALGORITMO_INGRESO_A_READY;
char* ALFA;
char* ESTIMACION_INICIAL;
char* TIEMPO_SUSPENSION;
char* LOG_LEVEL;

pthread_t hilo_conexion_cpu_dispatch;
pthread_t hilo_conexion_cpu_interrupt;
pthread_t hilo_conexion_io;

void inicializar_kernel(){
  /*
  logger_info = iniciar_logger("kernel.log", "Kernel", LOG_LEVEL_INFO);
  log_info(logger_info, "Logger Info de Kernel iniciado");
  logger_debug = iniciar_logger("kernel.log", "Kernel", LOG_LEVEL_DEBUG);
  log_debug(logger_debug, "Logger Debug de Kernel iniciado");
  */
  logger = iniciar_logger("kernel.log", "Kernel", LOG_LEVEL_INFO);
  log_info(logger, "Log de Kernel iniciado en 'init'");
  config = iniciar_config("kernel.config");
};

void extraer_datos_config(){
  IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
  PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
  PUERTO_ESCUCHA_DISPATCH = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
  PUERTO_ESCUCHA_IO = config_get_string_value(config, "PUERTO_ESCUCHA_IO");
  log_info(logger, "Datos extraídos del archivo de configuración");
};

void crear_hilos(){
  pthread_create(&hilo_conexion_cpu_dispatch, NULL, conectar_cpu_dispatch, PUERTO_ESCUCHA_DISPATCH);
  pthread_create(&hilo_conexion_io, NULL, conectar_io, PUERTO_ESCUCHA_IO);
  log_info(logger, "Creación de hilos realizada");
};

void unir_hilos(){
  log_info(logger, "Unión de hilos a punto de realizar");
  pthread_join(hilo_conexion_cpu_dispatch, NULL);
  log_info(logger, "---");
  pthread_join(hilo_conexion_io, NULL);
  log_info(logger, "Unión de hilos realizada");
};