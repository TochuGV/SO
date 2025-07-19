#include "entrante.h"

pthread_mutex_t mutex_conexion_cpu = PTHREAD_MUTEX_INITIALIZER;

int recibir_handshake_kernel(int cliente_kernel){
  int32_t handshake;
  int32_t ok = 0;
  int32_t error = -1;
  if(recv(cliente_kernel, &handshake, sizeof(int32_t), MSG_WAITALL) <= 0){
    log_error(logger, "Error recibiendo el tipo de módulo en Handshake");
    return -1;
  };

  switch(handshake){
    case MODULO_CPU:
      int32_t id_cpu;
      int32_t tipo_conexion;
      if (recv(cliente_kernel, &id_cpu, sizeof(int32_t), MSG_WAITALL) <= 0 || 
        recv(cliente_kernel, &tipo_conexion, sizeof(int32_t), MSG_WAITALL) <= 0 ||
        id_cpu <= 0 || (tipo_conexion != CPU_DISPATCH && tipo_conexion != CPU_INTERRUPT)){
        log_error(logger, "Handshake CPU inválido");
        send(cliente_kernel, &error, sizeof(int32_t), 0);
        return -1;
      };
      
      pthread_mutex_lock(&mutex_conexion_cpu);

      registrar_cpu_si_no_existe(id_cpu);
      t_cpu* cpu = obtener_cpu_por_id(id_cpu);

      if(tipo_conexion == CPU_DISPATCH){
        if(cpu->socket_dispatch != -1){
          log_error(logger, "CPU %d ya tiene conexión Dispatch", id_cpu);
          send(cliente_kernel, &error, sizeof(int32_t), 0);
          return -1;
        };
        cpu->socket_dispatch = cliente_kernel;
        //log_debug(logger, "CPU %d conectó Dispatch", id_cpu);
      } else {
        if(cpu->socket_interrupt != -1){
          log_error(logger, "CPU %d ya tiene una conexión Interrupt", id_cpu);
          send(cliente_kernel, &error, sizeof(int32_t), 0);
          return -1;
        };
        cpu->socket_interrupt = cliente_kernel;
        //log_debug(logger, "CPU %d conectó Interrupt", id_cpu);
      };
      send(cliente_kernel, &ok, sizeof(int32_t), 0);

      if(cpu_esta_completa(cpu)){
        //int valor_antes;
        //sem_getvalue(&semaforo_cpu_libre, &valor_antes);
        //log_debug(logger, "[entrante.c - Antes de sem_post] Semáforo CPU libre: %d", valor_antes);
        
        sem_post(&semaforo_cpu_libre);

        //int valor_despues;
        //sem_getvalue(&semaforo_cpu_libre, &valor_despues);
        //log_debug(logger, "[entrante.c - Después de sem_post] Semáforo CPU libre: %d", valor_despues);
        //log_debug(logger, "CPU %d está completamente conectada. Se habilita para planificación", id_cpu);
      };

      pthread_mutex_unlock(&mutex_conexion_cpu);
      return MODULO_CPU;
    case MODULO_IO:
      int32_t token_io;
      if(recv(cliente_kernel, &token_io, sizeof(int32_t), MSG_WAITALL) <= 0){
        log_error(logger, "Handshake IO inválido");
        send(cliente_kernel, &error, sizeof(int32_t), 0);
        return -1;
      };
      char* nombre_io = token_io_to_string(token_io);
      if(nombre_io == NULL){
        log_warning(logger, "Tipo de IO desconocido");
        send(cliente_kernel, &error, sizeof(int32_t), 0);
        return -1;
      };
      registrar_socket_io(nombre_io, cliente_kernel);
      send(cliente_kernel, &ok, sizeof(int32_t), 0);
      //log_debug(logger, "Dispositivo '%s' conectado", nombre_io);
      return MODULO_IO;
    default:
      send(cliente_kernel, &error, sizeof(int32_t), 0);
      log_warning(logger, "Tipo de módulo desconocido en Handshake: %d", handshake);
      break;
  };
  return -1;
};
