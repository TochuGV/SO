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

t_list* lista_cpus;
t_list* lista_pcbs;

pthread_mutex_t mutex_pcbs = PTHREAD_MUTEX_INITIALIZER;

pthread_t hilo_conexion_cpu_dispatch;
pthread_t hilo_conexion_cpu_interrupt;
pthread_t hilo_conexion_io;
pthread_t hilo_planificacion;

int conexion_memoria;

void inicializar_kernel(){
  logger = iniciar_logger("kernel.log", "Kernel", LOG_LEVEL_DEBUG);
  log_debug(logger, "Log de Kernel iniciado");
  config = iniciar_config("kernel.config");
  extraer_datos_config();
  lista_cpus = list_create();
  lista_pcbs = list_create();
  pthread_mutex_init(&mutex_pcbs, NULL);
  iniciar_planificacion_largo_plazo();
  iniciar_planificacion_corto_plazo();

  pthread_create(&hilo_planificacion, NULL, planificador_ciclo_general, NULL);
};

void extraer_datos_config(){
  IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
  PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
  PUERTO_ESCUCHA_DISPATCH = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
  PUERTO_ESCUCHA_INTERRUPT = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");
  PUERTO_ESCUCHA_IO = config_get_string_value(config, "PUERTO_ESCUCHA_IO");
  ALGORITMO_CORTO_PLAZO = config_get_string_value(config, "ALGORITMO_CORTO_PLAZO");
  ALGORITMO_INGRESO_A_READY = config_get_string_value(config, "ALGORITMO_INGRESO_A_READY");
  ALFA = config_get_string_value(config, "ALFA");
  ESTIMACION_INICIAL = config_get_string_value(config, "ESTIMACION_INICIAL");
  TIEMPO_SUSPENSION = config_get_string_value(config, "TIEMPO_SUSPENSION");
  //LOG_LEVEL = config_get_string_value(config, "LOG_LEVEL");
  log_debug(logger, "Datos extraídos del archivo de configuración");
};

/*
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
*/