#include "conexiones.h"

char* ip_kernel;
char* ip_memoria;
char* puerto_kernel_dispatch;
char* puerto_kernel_interrupt;
char* puerto_memoria;
int conexion_kernel_dispatch;
int conexion_kernel_interrupt;
int conexion_memoria;
datos_conexion_t* datos_dispatch;
datos_conexion_t* datos_interrupt;
datos_conexion_t* datos_memoria;

void iniciar_cpu(int32_t identificador_cpu) {

  ip_kernel = config_get_string_value(config, "IP_KERNEL");
  ip_memoria = config_get_string_value(config, "IP_MEMORIA");

  puerto_kernel_dispatch = config_get_string_value(config, "PUERTO_KERNEL_DISPATCH");
  puerto_kernel_interrupt = config_get_string_value(config, "PUERTO_KERNEL_INTERRUPT");
  puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");

  datos_dispatch = malloc(sizeof(datos_conexion_t));
  datos_dispatch->ip = ip_kernel;
  datos_dispatch->puerto = puerto_kernel_dispatch;
  datos_dispatch->id_cpu = identificador_cpu;
  datos_dispatch->socket = conexion_kernel_dispatch;

  datos_interrupt = malloc(sizeof(datos_conexion_t));
  datos_interrupt->ip = ip_kernel;
  datos_interrupt->puerto = puerto_kernel_interrupt;
  datos_interrupt->id_cpu = identificador_cpu;
  datos_interrupt->socket = conexion_kernel_interrupt;

  datos_memoria = malloc(sizeof(datos_conexion_t));
  datos_memoria->ip = ip_memoria;
  datos_memoria->puerto = puerto_memoria;
  datos_memoria->id_cpu = identificador_cpu;
  datos_memoria->socket = conexion_memoria;

  parametros_tlb = malloc(sizeof(tlb_t));
  parametros_tlb->cantidad_entradas = config_get_int_value(config, "ENTRADAS_TLB");
  parametros_tlb->algoritmo_reemplazo = config_get_string_value(config,"REEMPLAZO_TLB");

  tlb = malloc(sizeof(estructura_tlb) * parametros_tlb->cantidad_entradas);
  for (int i = 0; i < parametros_tlb->cantidad_entradas; i++) {
    tlb[i].pid = -1;
    tlb[i].pagina = -1;
    tlb[i].marco = -1;
    tlb[i].tiempo_transcurrido = 0;
  }
}

//Conexiones
void* conectar_dispatch(void* arg) {
  datos_conexion_t* datos = (datos_conexion_t*) arg;
    datos->socket = crear_conexion(datos->ip, datos->puerto, MODULO_CPU);
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
    conexion_kernel_dispatch = datos->socket;
    free(datos);
    return NULL;
}

void* conectar_interrupt(void* arg) {
  datos_conexion_t* datos = (datos_conexion_t*) arg;
    datos->socket = crear_conexion(datos->ip, datos->puerto, MODULO_CPU);
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
    conexion_kernel_interrupt = datos->socket;
    free(datos);
    return NULL;
}

void* conectar_memoria(void* arg) {
  datos_conexion_t* datos = (datos_conexion_t*) arg;
    datos->socket = crear_conexion(datos->ip, datos->puerto, MODULO_CPU);
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
    conexion_memoria = datos->socket;

    recibir_datos_memoria();

    free(datos);
    return NULL;
}

void recibir_datos_memoria() {

  int cod_op = DATOS_MEMORIA;
  send(conexion_memoria, &cod_op, sizeof(int), 0);

  cod_op = recibir_operacion(conexion_memoria);

  if (cod_op == DATOS_MEMORIA) {
    t_list* datos_memoria = recibir_paquete(conexion_memoria);

    tamanio_pagina = *(uint32_t*) list_get(datos_memoria, 0);
    cant_entradas_tabla = *(uint32_t*) list_get(datos_memoria, 1);
    cant_niveles = *(uint32_t*) list_get(datos_memoria, 2);
  }
}