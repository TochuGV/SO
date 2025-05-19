#include "common_cpu.h"

 char* ip_kernel;
 char* ip_memoria;
 char* puerto_kernel_dispatch;
 char* puerto_kernel_interrupt;
 char* puerto_memoria;
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

  datos_interrupt = malloc(sizeof(datos_conexion_t));
  datos_interrupt->ip = ip_kernel;
  datos_interrupt->puerto = puerto_kernel_interrupt;
  datos_interrupt->id_cpu = identificador_cpu;

  datos_memoria = malloc(sizeof(datos_conexion_t));
  datos_memoria->ip = ip_memoria;
  datos_memoria->puerto = puerto_memoria;
  datos_memoria->id_cpu = identificador_cpu;
}

//Conexiones
void* conectar(void* arg) {
  datos_conexion_t* datos = (datos_conexion_t*) arg;
    int socket = crear_conexion(datos->ip, datos->puerto, CPU);
    if (socket == -1) {
        log_error(logger, "Fallo al conectar");
        pthread_exit(NULL);
    }

    int32_t handshake_header = CPU;
    int32_t respuesta;
    int32_t identificador = datos->id_cpu;
    send(socket, &handshake_header, sizeof(int32_t), 0);
    recv(socket, &respuesta, sizeof(int32_t), MSG_WAITALL);
    send(socket, &identificador, sizeof(int32_t), 0);

    if (respuesta <= 0) {
      log_error(logger, "Fallo al recibir respuesta del handshake");
      close(socket);
      pthread_exit(NULL);
    }

    log_info(logger, "Conexión exitosa");

    free(datos);

    return NULL;
}

void* ciclo_de_instruccion(int conexion_kernel_dispatch,int conexion_memoria) {
  t_pcb* pcb; 
  t_list* lista_instruccion;
  t_instruccion instruccion;
  t_resultado_ejecucion estado;
  // Paso 1: recibir el PCB desde Kernel
  pcb = recibir_pcb(conexion_kernel_dispatch);
  if (pcb == NULL) return NULL;

  while(1){
    //Log Fetch instrucción
    log_info (logger, "## PID: %d - FETCH - Program Counter: %d", pcb->pid,pcb->pc);
    
    // Paso 2: obtener la instrucción desde Memoria
    lista_instruccion = recibir_instruccion(pcb, conexion_memoria);
    instruccion->tipo = *(t_instruccion*)list_get(lista_instruccion, 0);
    instruccion->parametro1 = *(t_instruccion*)list_get(lista_instruccion, 1);
    instruccion->parametro2 = *(t_instruccion*)list_get(lista_instruccion, 2);

    list_destroy_and_destroy_elements(lista_instruccion,free);
    // Paso 3: interpretar y ejecutar instrucción
    estado = trabajar_instruccion(instruccion,pcb);
    if (estado!=EJECUCION_CONTINUA){
      break;
    }
  }
  // Paso 4: devolver PCB actualizado a Kernel

  // Paso 5: liberar memoria
  free(pcb);  
  //Falta liberar/destruir la lista

  return NULL;
}

//Recibir información del PCB desde Kernel
t_pcb* recibir_pcb(int conexion_kernel_dispatch) {
  void* buffer = malloc(sizeof(uint32_t)*2);

  int bytes_recibidos= recv(conexion_kernel_dispatch,buffer,sizeof(uint32_t)*2,MSG_WAITALL);
  if (bytes_recibidos <= 0) {
    log_info(logger,"Fallo al recibir instrucción");
    free (buffer);
    return NULL;
  }

  t_pcb* pcb = deserializar_pcb(buffer);
  free (buffer);
  return pcb;
}

t_pcb* deserializar_pcb(void* buffer) {
  t_pcb* pcb = malloc(sizeof(t_pcb));
  int offset = 0;

  memcpy(&(pcb->pid), buffer + offset, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  memcpy(&(pcb->pc), buffer + offset, sizeof(uint32_t));
  offset += sizeof(uint32_t);
  
  return pcb;
}

//Solicitar instrucción a Memoria
t_list* recibir_instruccion(t_pcb* pcb, int conexion_memoria) {

  send(conexion_memoria, &(pcb->pc), sizeof(uint32_t), 0);
  send(conexion_memoria, & (pcb->pc), sizeof(uint32_t), 0);

  t_list* lista_instrucciones;
  lista_instrucciones=recibir_paquete(conexion_memoria);

  return lista_instrucciones;
}


//Decode y Execute Instrucción
t_resultado_ejecucion trabajar_instruccion (t_instruccion instruccion, t_pcb* pcb) {
  switch (instruccion.tipo) {
  //El log_info en cada Case corresponde a Instrucción Ejecutada
    case NOOP:
         log_info (logger, "## PID: %d - Ejecutando: NOOP", pcb->pid);
         ejecutar_noop();
         pcb->pc++;
         return EJECUCION_CONTINUA;
         break;
    
    case READ :
         log_info(logger, "## PID: %d - Ejecutando: READ - Direccion logica: %d - tamaño: %d", pcb->pid, instruccion.parametro1, instruccion.parametro2);
         ejecutar_read(instruccion.parametro1, instruccion.parametro2);
         pcb->pc++;
         return EJECUCION_CONTINUA;
         break;
    
    case WRITE : 
         log_info(logger, "## PID: %d - Ejecutando: WRITE - Direccion logica: %d - Valor: %d ", pcb->pid, instruccion.parametro1, instruccion.parametro2);
         ejecutar_write(instruccion.parametro1, instruccion.parametro2);
         pcb->pc++;
         return EJECUCION_CONTINUA;
         break;
    
    case GOTO: 
         log_info(logger, "## PID: %d - Ejecutando: GOTO - Nuevo PC: %d", pcb->pid, instruccion.parametro1);
         pcb->pc=instruccion.parametro1;
         return EJECUCION_CONTINUA;
         break;
    
    case IO: 
         log_info(logger, "## PID: %d - Ejecutando: IO - Dispositivo: %d - Tiempo: %d", pcb->pid, instruccion.parametro1, instruccion.parametro2);
         ejecutar_io(instruccion.parametro1, instruccion.parametro2);
         pcb->pc++;
         return EJECUCION_CONTINUA;
         break;
    
    case INIT_PROC:
         log_info(logger, "## PID: %d - Ejecutando: INIT_PROC - Archivo: %d - Tamaño: %d", pcb->pid, instruccion.parametro1, instruccion.parametro2);
         ejecutar_init_proc(instruccion.parametro1, instruccion.parametro2);
         pcb->pc++;
         return EJECUCION_CONTINUA;
         break;

    case DUMP_MEMORY:
         log_info(logger,"## PID: %d - Ejecutando: DUMP_MEMORY", pcb->pid);
         ejecutar_dump_memory();
         pcb->pc++;
         return EJECUCION_CONTINUA;
         break;

    case EXIT:
         log_info (logger, "## PID: %d- Ejecutando: EXIT", pcb->pid);
         ejecutar_exit();
         pcb->pc++;
         return EJECUCION_FINALIZADA;
         break;
    
    default: 
    log_warning(logger, "## PID: %d - Instruccion desconocida: %d", pcb->pid, instruccion.tipo);
     break;
     return EJECUCION_FINALIZADA;
   }
}



