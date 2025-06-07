#include "common_cpu.h"

//bool interrupción_activa = false; 
//pthread_mutex_t mutex_interrupcion = PTHREAD_MUTEX_INITIALIZER; No hace falta por ahora

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
  estructura_tlb* tlb;
  tlb_t* parametros_tlb;
  
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
    datos->socket = crear_conexion(datos->ip, datos->puerto, CPU);
    if (datos->socket == -1) {
        log_error(logger, "Fallo al conectar con Kernel Dispatch");
        pthread_exit(NULL);
    }

    int32_t handshake_header = CPU;
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
    datos->socket = crear_conexion(datos->ip, datos->puerto, CPU);
    if (datos->socket == -1) {
        log_error(logger, "Fallo al conectar con Kernel Interrupt");
        pthread_exit(NULL);
    }

    int32_t handshake_header = CPU;
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

    free(datos);

    return NULL;
}

void* conectar_memoria(void* arg) {
  datos_conexion_t* datos = (datos_conexion_t*) arg;
    datos->socket = crear_conexion(datos->ip, datos->puerto, CPU);
    if (datos->socket == -1) {
        log_error(logger, "Fallo al conectar con Memoria");
        pthread_exit(NULL);
    }

    int32_t handshake_header = CPU;
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

    free(datos);

    return NULL;
}

//Realizar todo el ciclo de cada instrucción
void* ciclo_de_instruccion(t_pcb* pcb, int conexion_kernel_dispatch,int conexion_memoria,int conexion_kernel_interrupt) {
  t_list* lista_instruccion;
  t_instruccion instruccion;
  t_estado_ejecucion estado=EJECUCION_CONTINUA;

  while(estado==EJECUCION_CONTINUA){
    //Log 1.  Fetch instrucción
    log_info (logger, "## PID: %d - FETCH - Program Counter: %d", pcb->pid,pcb->pc);
    
    // Paso 2: obtener la instrucción desde Memoria
    lista_instruccion = recibir_instruccion(pcb, conexion_memoria);
    if (list_size(lista_instruccion) == 0) {
      log_warning(logger, "No se recibió ninguna instrucción desde Memoria");
    }
    instruccion.parametro1 = *(int*)list_get(lista_instruccion, 0);
    instruccion.parametro1 = *(uint32_t*)list_get(lista_instruccion, 1);
    instruccion.parametro2 = *(uint32_t*)list_get(lista_instruccion, 2);

    list_destroy_and_destroy_elements(lista_instruccion,free);
    // Paso 3: interpretar y ejecutar instrucción
    estado = trabajar_instruccion(instruccion,pcb);
    // Paso 4: Chequear interrupción
    if (chequear_interrupcion(conexion_kernel_interrupt, pcb->pid)) {
      log_info(logger, "PID %d interrumpido", pcb->pid);
      estado = EJECUCION_BLOQUEADA_SOLICITUD;
    }
  }
  // Paso 5: devolver PCB actualizado a Kernel
  actualizar_kernel(instruccion,estado,pcb,conexion_kernel_dispatch);
  // Paso 6: liberar memoria
  free(pcb);

  return NULL;
}

//Recibir información del PCB desde Kernel
t_pcb* recibir_pcb(int conexion_kernel_dispatch) {
  int cod_op = recibir_operacion(conexion_kernel_dispatch);
  if(cod_op != PCB) {
    log_error(logger, "ERROR");
    return NULL;
  };
  
  t_list* campos = recibir_paquete(conexion_kernel_dispatch);
  if(list_size(campos) == 0) return NULL;
  void* buffer = list_get(campos, 0);
  t_pcb* pcb = deserializar_pcb(buffer);
  list_destroy_and_destroy_elements(campos, free);
  return pcb;
};

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

  send(conexion_memoria, &(pcb->pid), sizeof(uint32_t), 0);
  send(conexion_memoria, &(pcb->pc), sizeof(uint32_t), 0);

  t_list* lista_instrucciones;
  lista_instrucciones=recibir_paquete(conexion_memoria);

  return lista_instrucciones;
}

//Decode y Execute Instrucción
t_estado_ejecucion trabajar_instruccion (t_instruccion instruccion, t_pcb* pcb) {
  switch (instruccion.tipo) {
  //El log_info en cada Case corresponde a Log 3. Instrucción Ejecutada
    case NOOP:
         log_info (logger, "## PID: %d - Ejecutando: NOOP", pcb->pid);
         sleep(1); //Uso una duración de 1 segundo como default para NOOP
         pcb->pc++;
         return EJECUCION_CONTINUA;
         break;
    
    case READ :
         log_info(logger, "## PID: %d - Ejecutando: READ - Direccion logica: %d - tamaño: %d", pcb->pid, instruccion.parametro1, instruccion.parametro2);
         //ejecutar_read(pcb->pid,instruccion.parametro1, instruccion.parametro2);
         pcb->pc++;
         return EJECUCION_CONTINUA;
         break;
    
    case WRITE : 
         log_info(logger, "## PID: %d - Ejecutando: WRITE - Direccion logica: %d - Valor: %d ", pcb->pid, instruccion.parametro1, instruccion.parametro2);
        //ejecutar_write(pcb->pid,instruccion.parametro1, instruccion.parametro2);
         pcb->pc++;
         return EJECUCION_CONTINUA;
         break;
    
    case GOTO: 
         log_info(logger, "## PID: %d - Ejecutando: GOTO - Nuevo PC: %d", pcb->pid, instruccion.parametro1);
         pcb->pc=instruccion.parametro1;
         return EJECUCION_CONTINUA;
         break;
    
    case INSTRUCCION_IO: 
         log_info(logger, "## PID: %d - Ejecutando: IO - Dispositivo: %d - Tiempo: %d", pcb->pid, instruccion.parametro1, instruccion.parametro2);
         pcb->pc++;
         return EJECUCION_BLOQUEADA_IO;
         break;
    
    case INIT_PROC:
         log_info(logger, "## PID: %d - Ejecutando: INIT_PROC - Archivo: %d - Tamaño: %d", pcb->pid, instruccion.parametro1, instruccion.parametro2);
         //ejecutar_init_proc(instruccion.parametro1, instruccion.parametro2);
         pcb->pc++;
         return EJECUCION_BLOQUEADA_INIT_PROC;
         break;

    case DUMP_MEMORY:
         log_info(logger,"## PID: %d - Ejecutando: DUMP_MEMORY", pcb->pid);
         pcb->pc++;
         return EJECUCION_BLOQUEADA_DUMP;
         break;

    case EXIT:
         log_info(logger, "## PID: %d- Ejecutando: EXIT", pcb->pid);
         pcb->pc++;
         return EJECUCION_FINALIZADA;
         break;
    
    default: 
    log_warning(logger, "## PID: %d - Instruccion desconocida: %d", pcb->pid, instruccion.tipo);
     break;
     return EJECUCION_FINALIZADA;
   }
}
/*
//Ejecutar Write y Read
void ejecutar_read (uint32_t pid, uint32_t direccion_logica, uint32_t valor) {
  int direccion_fisica = traducir_direccion (pid, direccion_logica, valor);
  //Log 4. Lectura/Escritura Memoria
  log_info(logger, "PID: %d - Acción: LEER - Dirección Física: %d - Valor: %d", pid, direccion_fisica, valor);
}


void ejecutar_write(uint32_t pid, uint32_t direccion_logica, uint32_t valor) {
  int direccion_fisica = traducir_direccion (pid, direccion_logica, valor);
  //Log 4. Lectura/Escritura Memoria
  log_info(logger, "PID: %d - Acción: ESCRIBIR - Dirección Física: %d - Valor: %d", pid, direccion_fisica, valor);
}
*/

//Retornar a Kernel el PCB actualizado y el motivo de la interrupción
void actualizar_kernel(t_instruccion instruccion,t_estado_ejecucion estado,t_pcb* pcb,int conexion_kernel_dispatch){
  switch(estado){
    case EJECUCION_FINALIZADA:
    enviar_finalizacion(instruccion,estado,pcb,conexion_kernel_dispatch);
    break;

    case EJECUCION_BLOQUEADA_IO:
    enviar_bloqueo_IO(instruccion,estado,pcb,conexion_kernel_dispatch);
    break;

    case EJECUCION_BLOQUEADA_INIT_PROC:
    enviar_bloqueo_INIT_PROC(instruccion,estado,pcb,conexion_kernel_dispatch);
    break;

    case EJECUCION_BLOQUEADA_DUMP:
    enviar_bloqueo_DUMP(instruccion,estado,pcb,conexion_kernel_dispatch);
    break;

    default:
    log_warning(logger, "Motivo de interrupción desconocida");
  }
}

//Proceso finalizado
void enviar_finalizacion(t_instruccion instruccion,t_estado_ejecucion estado,t_pcb* pcb,int conexion_kernel_dispatch) {
  t_paquete* paquete = crear_paquete(SYSCALL_EXIT);
  llenar_paquete(paquete,estado,pcb);

  enviar_paquete(paquete, conexion_kernel_dispatch);
  eliminar_paquete(paquete);
} 

//Bloqueo por IO
void enviar_bloqueo_IO(t_instruccion instruccion,t_estado_ejecucion estado,t_pcb* pcb,int conexion_kernel_dispatch){
  t_paquete* paquete = crear_paquete(SYSCALL_IO);
  llenar_paquete(paquete,estado,pcb);
  agregar_a_paquete(paquete, &(instruccion.parametro1), sizeof(int)); // dispositivo
  agregar_a_paquete(paquete, &(instruccion.parametro2), sizeof(int)); // tiempo

  enviar_paquete(paquete, conexion_kernel_dispatch);
  eliminar_paquete(paquete);
}

//Bloqueo por INIT_PROC
void enviar_bloqueo_INIT_PROC(t_instruccion instruccion,t_estado_ejecucion estado,t_pcb* pcb,int conexion_kernel_dispatch){
  t_paquete* paquete = crear_paquete(SYSCALL_INIT_PROC);
  llenar_paquete(paquete,estado,pcb);
  agregar_a_paquete(paquete, &(instruccion.parametro1), sizeof(int)); // archivo
  agregar_a_paquete(paquete, &(instruccion.parametro2), sizeof(int)); // tamaño

  enviar_paquete(paquete, conexion_kernel_dispatch);
  eliminar_paquete(paquete);
}

//Bloqueo por Dump Memory
void enviar_bloqueo_DUMP(t_instruccion instruccion,t_estado_ejecucion estado,t_pcb* pcb,int conexion_kernel_dispatch){
  t_paquete* paquete = crear_paquete(SYSCALL_DUMP_MEMORY);
  llenar_paquete(paquete,estado,pcb);

  enviar_paquete(paquete, conexion_kernel_dispatch);
  eliminar_paquete(paquete);
}

//Funcion auxiliar para empaquetar PID, PC y tipo de interrupción
void llenar_paquete (t_paquete*paquete, t_estado_ejecucion estado,t_pcb* pcb){
  agregar_a_paquete(paquete, &(pcb->pid), sizeof(uint32_t)); //PID del proceso ejecutado
  agregar_a_paquete(paquete, &(pcb->pc), sizeof(uint32_t)); //PC actualizado
  agregar_a_paquete(paquete,&estado,sizeof(t_estado_ejecucion)); //Tipo de interrupción
}

//funcion que espera el mensaje de kernel
bool chequear_interrupcion(int socket_interrupt, uint32_t pid_actual) {
    int pid_interrupcion;
    int bytes = recv(socket_interrupt, &pid_interrupcion, sizeof(int), MSG_DONTWAIT);

    if (bytes > 0) {
      //Log 2. Interrupción Recibida
        log_info(logger, "LLega interrupción al puerto Interrupt %d", pid_interrupcion);
        if (pid_interrupcion == pid_actual) {
            return true;
        } else {
            log_info(logger, "PID %d no corresponde al proceso en ejecución (PID actual: %d)", pid_interrupcion, pid_actual);
        }
    }

    return false;
}
/*
//MMU
uint32_t traducir_direccion (uint32_t pid, uint32_t direccion_logica, uint32_t parametro) {
  int tamaño_pagina, cant_entradas_tabla, cant_niveles;

  //Obtener parametros desde memoria 
  send(socket_memoria, &TAMANIO_PAGINA, sizeof(int32_t), 0);
  recv(socket_memoria, &tamaño_pagina, sizeof(int32_t), MSG_WAITALL);
  send(socket_memoria, &CANT_ENTRADAS, sizeof(int32_t), 0);
  recv(socket_memoria, &cant_entradas_tabla, sizeof(int32_t), MSG_WAITALL);
  send(socket_memoria, &CANT_NIVELES, sizeof(int32_t), 0);
  recv(socket_memoria, &cant_niveles, sizeof(int32_t), MSG_WAITALL);

  //Calcular numero de pagina y desplazamiento 
  int nro_pagina = floor(direccion_logica / tamaño_pagina);
  int desplazamiento = direccion_logica % tamaño_pagina;
  int tabla_actual;

  int marco = consultar_TLB(nro_pagina);

  if (marco != -1) {
    //Log 6. TLB Hit
    log_info(logger, "PID: %d - TLB HIT - Pagina: %d", pid, nro_pagina);
    //Log 5. Obtener Marco
    log_info(logger, "PID: %d - OBTENER MARCO - Página: %d - Marco: %d", pid, nro_pagina, marco);
    return marco * tamaño_pagina + desplazamiento;
  }

  //Log 7. TLB Miss
  log_info(logger, "PID: %d - TLB MISS - Pagina: %d", pid, nro_pagina);

  Acá tiene que hacer todo el ciclo de revisar las páginas y tablas de memoria hasta tener una coincidencia
  Si no hay coincidencia, tiene que devolver Page Fault
    for (int nivel=0; nivel < cant_niveles; nivel++) {
    int entrada_nivel_X = floor(nro_pagina / pow(cant_entradas_tabla,cant_niveles - nivel - 1)) % cant_entradas_tabla;
    //Con la entrada del nivel actual, le pido a memoria el marco 
  
    send(socket_memoria, &tabla_actual, sizeof(int), 0);
    send(socket_memoria, &entrada_nivel, sizeof(int), 0);

    recv(socket_memoria, &tabla_actual, sizeof(int), MSG_WAITALL);
  }
  
  actualizar_TLB(pid, nro_pagina, marco);

  //Log 5. Obtener Marco
  log_info(logger, "PID: %d - OBTENER MARCO - Página: %d - Marco: %d", pid, nro_pagina, marco);
  return marco * tamaño_pagina + desplazamiento;
}
*/
// consultar TLB 
int consultar_TLB (uint32_t pid, int nro_pagina) {
  for (int i=0;i<parametros_tlb->cantidad_entradas ;i++) {
    if (tlb[i].pid==pid && tlb[i].pagina==nro_pagina) {
      return tlb[i].marco;
    }
  }
  return -1;
}

/*
//Insertar un nuevo marco en el TLB
void actualizar_TLB (uint32_t pid, int pagina, int marco) {
  
  switch (parametros_tlb->algoritmo_reemplazo){
    case "LRU":
    //Se fija cual es el que no usa hace más tiempo y lo reemplaza
    break;

    case "FIFO":
    //Se fija cual es el que entró hace más tiempo y lo reemplaza
    break;
  }
}
*/


/*
Por ahora no es necesario manejarnos con semáforos y esta versión no chequea el PID
bool escuchar_interrupt(int conexion_kernel_interrupt){
  int socket_interrupt= *((int*)socket_interrupt_ptr);
  int32_t mensaje;

  while(1){
    if (recv(socket_interrupt, &mensaje, sizeof(int32_t),MSG_WAITALL)>0){
      pthread_mutex_lock(&mutex_interrupcion);
      interrupcion_activa= true;
      pthread_mutex_unlock(&mutex_interrupcion);

      log_info(logger, "## Llega interrupcion al puerto Interrupt");
    }
  }
  return NULL;
}*/
