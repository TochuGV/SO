#include "conexiones.h"

const char* ip_kernel;
const char* ip_memoria;
const char* puerto_kernel_dispatch;
const char* puerto_kernel_interrupt;
const char* puerto_memoria;
uint32_t tamanio_pagina;
uint32_t cant_entradas_tabla;
uint32_t cant_niveles;

t_cpu* iniciar_cpu(int32_t identificador_cpu) {

  t_cpu* cpu = malloc(sizeof(t_cpu));
  cpu->id = identificador_cpu;

  cpu->datos_dispatch = malloc(sizeof(datos_conexion_t));
  cpu->datos_dispatch->ip = ip_kernel;
  cpu->datos_dispatch->puerto = puerto_kernel_dispatch;
  cpu->datos_dispatch->id_cpu = identificador_cpu;

  cpu->datos_interrupt = malloc(sizeof(datos_conexion_t));
  cpu->datos_interrupt->ip = ip_kernel;
  cpu->datos_interrupt->puerto = puerto_kernel_interrupt;
  cpu->datos_interrupt->id_cpu = identificador_cpu;

  cpu->datos_memoria = malloc(sizeof(datos_conexion_t));
  cpu->datos_memoria->ip = ip_memoria;
  cpu->datos_memoria->puerto = puerto_memoria;
  cpu->datos_memoria->id_cpu = identificador_cpu;

  cpu->parametros_cache=malloc(sizeof(cache_paginas_t));
  cpu->parametros_cache->cantidad_entradas = config_get_int_value(config,"ENTRADAS_CACHE");
  char* algoritmo_cache = config_get_string_value(config,"REEMPLAZO_CACHE");
  cpu->parametros_cache->algoritmo_reemplazo = convertir_cache (algoritmo_cache);

  cpu->parametros_tlb = malloc(sizeof(tlb_t));
  cpu->parametros_tlb->cantidad_entradas = config_get_int_value(config, "ENTRADAS_TLB");
  char* algoritmo_tlb = config_get_string_value(config,"REEMPLAZO_TLB");
  cpu->parametros_tlb->algoritmo_reemplazo = convertir_tlb (algoritmo_tlb);

  cpu->cache = malloc(sizeof(entrada_cache) * cpu->parametros_cache->cantidad_entradas);
  for (int i = 0; i < cpu->parametros_cache->cantidad_entradas; i++) {
    cpu->cache[i].pid = -1;
    cpu->cache[i].pagina = -1;
    cpu->cache[i].contenido = NULL;
    cpu->cache[i].bit_uso = 0;
    cpu->cache[i].bit_modificado = 0;
    cpu->cache[i].desplazamiento = 0;

  }

  cpu->tlb = malloc(sizeof(entrada_tlb) * cpu->parametros_tlb->cantidad_entradas);
  for (int i = 0; i < cpu->parametros_tlb->cantidad_entradas; i++) {
    cpu->tlb[i].pid = -1;
    cpu->tlb[i].pagina = -1;
    cpu->tlb[i].marco = -1;
    cpu->tlb[i].tiempo_transcurrido = 0;
  }

  cpu->orden_actual_tlb = 0;
  return cpu;
}

//Conexiones
void* conectar_dispatch(void* arg) {
  t_cpu* cpu = arg; 
  datos_conexion_t* datos = cpu->datos_dispatch;
    datos->socket = crear_conexion((char*)datos->ip, (char*)datos->puerto, MODULO_CPU);
    if (datos->socket == -1) {
        log_error(logger, "Fallo al conectar con Kernel Dispatch");
        pthread_exit(NULL);
    }

    int32_t handshake_header = MODULO_CPU;
    int32_t identificador = datos->id_cpu;
    int32_t tipo_conexion = CPU_DISPATCH;
    int32_t respuesta;
    send(datos->socket, &handshake_header, sizeof(int32_t), 0);
    send(datos->socket, &identificador, sizeof(int32_t), 0);
    send(datos->socket, &tipo_conexion, sizeof(int32_t), 0);
    recv(datos->socket, &respuesta, sizeof(int32_t), MSG_WAITALL);
    
    if (respuesta < 0) {
      log_error(logger, "Fallo al recibir respuesta del handshake con Kernel Dispatch");
      close(datos->socket);
      pthread_exit(NULL);
    }

    log_info(logger, "Conexión con Kernel Dispatch exitosa");
    cpu->conexion_kernel_dispatch = datos->socket;

    return NULL;
}

void* conectar_interrupt(void* arg) {
  t_cpu* cpu = arg; 
  datos_conexion_t* datos = cpu->datos_interrupt;
    datos->socket = crear_conexion((char*)datos->ip, (char*)datos->puerto, MODULO_CPU);
    if (datos->socket == -1) {
        log_error(logger, "Fallo al conectar con Kernel Interrupt");
        pthread_exit(NULL);
    }

    int32_t handshake_header = MODULO_CPU;
    int32_t identificador = datos->id_cpu;
    int32_t tipo_conexion = CPU_INTERRUPT;
    int32_t respuesta;
    send(datos->socket, &handshake_header, sizeof(int32_t), 0);
    send(datos->socket, &identificador, sizeof(int32_t), 0);
    send(datos->socket, &tipo_conexion, sizeof(int32_t), 0);
    recv(datos->socket, &respuesta, sizeof(int32_t), MSG_WAITALL);

    if (respuesta < 0) {
      log_error(logger, "Fallo al recibir respuesta del handshake con Kernel Interrupt");
      close(datos->socket);
      pthread_exit(NULL);
    }

    log_info(logger, "Conexión con Kernel Interrupt exitosa");
    cpu->conexion_kernel_interrupt = datos->socket;

    return NULL;
}

void* conectar_memoria(void* arg) {
  t_cpu* cpu = arg; 
  datos_conexion_t* datos = cpu->datos_memoria;
    datos->socket = crear_conexion((char*)datos->ip, (char*)datos->puerto, MODULO_CPU);
    if (datos->socket == -1) {
        log_error(logger, "Fallo al conectar con Memoria");
        pthread_exit(NULL);
    }

    int32_t handshake_header = MODULO_CPU;
    int32_t identificador = datos->id_cpu;
    int32_t respuesta;
    send(datos->socket, &handshake_header, sizeof(int32_t), 0);
    send(datos->socket, &identificador, sizeof(int32_t), 0);
    recv(datos->socket, &respuesta, sizeof(int32_t), MSG_WAITALL);

    if (respuesta < 0) {
      log_error(logger, "Fallo al recibir respuesta del handshake con Memoria");
      close(datos->socket);
      pthread_exit(NULL);
    }

    log_info(logger, "Conexión con Memoria exitosa");
    cpu->conexion_memoria = datos->socket;

    recibir_datos_memoria(cpu);

    return NULL;
}

void recibir_datos_memoria(t_cpu* cpu) {

  int cod_op = DATOS_MEMORIA;
  send(cpu->conexion_memoria, &cod_op, sizeof(int), 0);

  cod_op = recibir_operacion(cpu->conexion_memoria);

  if (cod_op == DATOS_MEMORIA) {
    t_list* datos_memoria = recibir_paquete(cpu->conexion_memoria);

    tamanio_pagina = *(uint32_t*) list_get(datos_memoria, 0);
    cant_entradas_tabla = *(uint32_t*) list_get(datos_memoria, 1);
    cant_niveles = *(uint32_t*) list_get(datos_memoria, 2);
  }
}

//Auxiliar para convertir valores del config
t_algoritmo_tlb convertir_tlb(char* algoritmo) {
    if (strcmp(algoritmo, "LRU") == 0) {return LRU;}
    if (strcmp(algoritmo, "FIFO") == 0) {return FIFO;}
    return 0;
  } 

t_algoritmo_cache convertir_cache (char* algoritmo) {
  if (strcmp(algoritmo, "CLOCK") == 0) {return CLOCK;} 
  if (strcmp(algoritmo, "CLOCK-M") == 0) {return CLOCK_M;}
  return 0;
  } 

//Auxiliar para liberar los espacios de memoria reservados
void liberar_cpu(t_cpu* cpu) {
    free(cpu->datos_dispatch);
    free(cpu->datos_interrupt);
    free(cpu->datos_memoria);

    free(cpu->parametros_cache);
    free(cpu->parametros_tlb);

    free(cpu->cache);
    free(cpu->tlb);

    free(cpu);
}
