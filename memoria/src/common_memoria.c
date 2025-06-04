#include "common_memoria.h"

uint32_t tamanio_memoria;
void* memoria;
uint32_t memoria_usada;
char* path_instrucciones;

int servidor_memoria;
char* puerto_escucha;

t_list* lista_procesos;
char* NOMBRES_INSTRUCCIONES[] = {
  "NOOP",
  "WRITE",
  "READ",
  "GOTO",
  "INSTRUCCION_IO",
  "INIT_PROC",
  "DUMP_MEMORY",
  "EXIT"
};

void inicializar_memoria(void)
{
  config = iniciar_config("memoria.config");
  logger = iniciar_logger("memoria.log", "Memoria", LOG_LEVEL_DEBUG);
  log_debug(logger, "Memoria iniciada :D!");

  puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
  tamanio_memoria = config_get_int_value(config, "TAM_MEMORIA");
  path_instrucciones = config_get_string_value(config, "PATH_INSTRUCCIONES");

  memoria = malloc(tamanio_memoria);
  memoria_usada = 0;

  servidor_memoria = iniciar_servidor(puerto_escucha);

  lista_procesos = list_create();
}

// Para memoria nomas necesito saber si la conexion es Kernel o de CPU, por eso un handshake tan basico
void* atender_cliente(void* arg){
  int cliente_memoria = *(int*)arg;
  log_debug(logger, "Cliente aceptado con socket FD: %d", cliente_memoria);
  free(arg);
  pthread_t hilo_atender;
  int32_t cliente = recibir_handshake_memoria(cliente_memoria);

  int* socket_ptr = malloc(sizeof(int));
  *socket_ptr = cliente_memoria;

  if (cliente == KERNEL) {
    log_info(logger, "## Kernel Conectado - FD del socket: <%d>",cliente);
    pthread_create(&hilo_atender, NULL, atender_kernel, socket_ptr);
    pthread_detach(hilo_atender);
  }
  else if (cliente == CPU) {
    pthread_create(&hilo_atender, NULL, atender_cpu, socket_ptr);
    pthread_join(hilo_atender,NULL);
  }
  else {
    log_warning(logger, "Handshake desconocido");
  }
  pthread_detach(pthread_self());
  return NULL;
};

int recibir_handshake_memoria(int cliente_memoria)
{
  int32_t handshake;
  int32_t resultado_ok = 0;
  int32_t resultado_error = -1;

 // recv(cliente_memoria, &handshake, sizeof(int32_t), MSG_WAITALL);
  
if (recv(cliente_memoria, &handshake, sizeof(int32_t), MSG_WAITALL) <= 0) {
    log_error(logger, "Error recibiendo handshake de Kernel. Cerrando conexión...");
    close(cliente_memoria);
    return -1;
}

  switch (handshake)
  {
  case KERNEL:
    log_debug(logger, "Memoria responde OK al Kernel");
    send(cliente_memoria, &resultado_ok, sizeof(int32_t), 0);
    return KERNEL;
    break;

  case CPU:
    int32_t identificador_cpu;
    send(cliente_memoria, &resultado_ok, sizeof(int32_t), 0);
    recv(cliente_memoria, &identificador_cpu, sizeof(int32_t),MSG_WAITALL);
    log_debug(logger, "CPU %d conectada.", identificador_cpu);
    return CPU;
    break;
  
  default:
    send(cliente_memoria, &resultado_error, sizeof(int32_t), 0);
    break;
  }
  return -1;
}

void* atender_kernel(void* arg){
  int cliente_kernel = *(int*)arg;
  log_debug(logger, "Entrando a atender_kernel() con socket FD: %d", cliente_kernel);

    t_list* lista;

    while (1) {
      log_debug(logger, "Esperando operación de Kernel...");
      int cod_op = recibir_operacion(cliente_kernel);
      log_debug(logger, "Código de operación recibido: %d", cod_op);
      switch (cod_op) {
      case PAQUETE:
        lista = recibir_paquete(cliente_kernel);
        list_iterate(lista, (void*) iterator);
        break;

      case SOLICITUD_MEMORIA:

        int32_t resultado_ok = 0;
        int32_t resultado_error = -1;
        int resultado = recibir_y_ubicar_proceso(cliente_kernel);
        printf("%d", resultado);
        if (resultado == 0) {
          send(cliente_kernel,&resultado_ok,sizeof(int32_t),0);
        }
        else
          send(cliente_kernel,&resultado_error,sizeof(int32_t),0);
        break;
        
      case -1:
        log_error(logger, "Kernel se desconectó. Terminando servidor...");
        pthread_exit((void*)EXIT_FAILURE);
      default:
        log_warning(logger,"Operación desconocida. No quieras meter la pata.");
        break;
      };
    };
  return NULL;
};

void* atender_cpu(void* arg)
{
  int cliente_cpu = *(int*)arg;

    t_list* lista;
    while (1) {
      int cod_op = recibir_operacion(cliente_cpu);
      switch (cod_op) {
      case PAQUETE:
        lista = recibir_paquete(cliente_cpu);
        list_iterate(lista, (void*) iterator);
        break;
      case SOLICITUD_INSTRUCCION:
        recibir_solicitud_instruccion(cliente_cpu);
        break;
      case -1:
        log_error(logger, "CPU se desconectó. Terminando servidor...");
        pthread_exit((void*)EXIT_FAILURE);
      default:
        log_warning(logger,"Operación desconocida. No quieras meter la pata.");
        break;
      }
    }
  return NULL;
}

void* recibir_solicitud_instruccion(int cliente_cpu)
{
  uint32_t pid;
  uint32_t pc;
  t_proceso* proceso;
  t_instruccion* instruccion;

  recv(cliente_cpu,&pid,sizeof(int32_t),MSG_WAITALL);
  recv(cliente_cpu,&pc,sizeof(int32_t),MSG_WAITALL);

  for(int i = 0; i<list_size(lista_procesos);i++) {

    proceso = list_get(lista_procesos,i);

    if (proceso->pid == pid) {

      if (pc >= list_size(proceso->lista_instrucciones)) {
      log_error(logger, "El PC %d excede la cantidad de instrucciones del proceso %d.", pc, pid);
      return NULL;
      }

      instruccion = list_get(proceso->lista_instrucciones,pc);

      t_tipo_instruccion tipo = instruccion->tipo;
      uint32_t parametro1 = instruccion->parametro1;
      uint32_t parametro2 = instruccion->parametro2;

      log_info(logger, "## PID: <%d> - Obtener instrucción: <%d> - Instrucción: <%s> <%d  %d>",pid,pc,NOMBRES_INSTRUCCIONES[tipo],parametro1,parametro2);

      t_paquete* paquete = crear_paquete(INSTRUCCION);

      agregar_a_paquete(paquete, &tipo, sizeof(tipo));
      agregar_a_paquete(paquete, &parametro1, sizeof(uint32_t));
      agregar_a_paquete(paquete, &parametro2, sizeof(uint32_t));

      enviar_paquete(paquete, cliente_cpu);

      log_debug(logger,"Proceso: %d; Instruccion numero: %d enviada a CPU.",pid,pc);

      return NULL;
    }
  }

  log_warning(logger,"Error al encontrar el proceso que CPU solicita.");
  return NULL;
}

bool verificar_espacio_memoria(uint32_t tamanio_proceso)
{
  if(memoria_usada + tamanio_proceso > tamanio_memoria)
    return false;
  
  return true;
}

t_list* leer_archivo_instrucciones(char* archivo_pseudocodigo)
{
  char path_archivo[256];
  snprintf(path_archivo, sizeof(path_archivo), "%s%s", path_instrucciones, archivo_pseudocodigo);
  FILE *file = fopen(path_archivo, "r");
  log_debug(logger, "Abriendo archivo de instrucciones: %s", path_archivo);
  if (file == NULL) {
    log_error(logger, "Error: no se pudo ingresar al proceso");
    return NULL;
  }

  t_list* lista_instrucciones = list_create();
  t_instruccion instruccion;
  char linea[256];

  while ((fgets(linea, sizeof(linea), file) != NULL)) {
    char* token = strtok(linea, " \n");

    for(int indice_intruccion = 0; indice_intruccion < CANTIDAD_INSTRUCCIONES; indice_intruccion++) {
      if (strcmp(token, NOMBRES_INSTRUCCIONES[indice_intruccion]) == 0) {
        instruccion.tipo = indice_intruccion;
        break;
      }
    }

    char* param1 = strtok(NULL, " \n");
    char* param2 = strtok(NULL, " \n");

    if (param1 != NULL)
      instruccion.parametro1 = atoi(param1);
    else
      instruccion.parametro1 = 0;

    if (param2 != NULL)
      instruccion.parametro2 = atoi(param2);
    else
      instruccion.parametro2 = 0;
    
    t_instruccion* nueva_instruccion = malloc(sizeof(t_instruccion));
    *nueva_instruccion = instruccion;

    list_add(lista_instrucciones, nueva_instruccion);
  }

  return lista_instrucciones;
}

int recibir_y_ubicar_proceso(int cliente_kernel)
{
  t_list* valores = recibir_paquete(cliente_kernel);
  if (!valores || list_size(valores) < 4) {
    log_error(logger, "Paquete recibido de Kernel es inválido o incompleto");
    return -1;
  }
  uint32_t tamanio_proceso = *(uint32_t*)list_get(valores, 2); 
  printf("Hola");
  if (verificar_espacio_memoria(tamanio_proceso)) {
    uint32_t longitud_archivo_pseudocodigo = *(int32_t*)list_get(valores, 0); 
    char* archivo_pseudocodigo = malloc(longitud_archivo_pseudocodigo);
    memcpy(archivo_pseudocodigo, list_get(valores, 1), longitud_archivo_pseudocodigo);
    archivo_pseudocodigo[longitud_archivo_pseudocodigo - 1] = '\0';
    log_debug(logger, "Nombre del archivo recibido: '%s' (longitud %d)", archivo_pseudocodigo, longitud_archivo_pseudocodigo);

    uint32_t pid = *(uint32_t*)list_get(valores, 3);

    log_debug(logger,"Proceso %s con pid %d recibido",archivo_pseudocodigo,pid);

    t_proceso* proceso = malloc(sizeof(t_proceso));
    proceso->pid = pid;
    log_debug(logger, "Intentando leer archivo: '%s'", archivo_pseudocodigo);
    proceso->lista_instrucciones = leer_archivo_instrucciones(archivo_pseudocodigo);

    list_add(lista_procesos,proceso);

    log_info(logger, "## PID: <%d> - Proceso Creado - Tamaño: <%d>",pid,tamanio_proceso);

    return 0;
  };

  log_warning(logger,"No hay lugar en memoria para el proceso.");
  return -1;
}


void terminar_memoria(void)
{
  close(servidor_memoria);
	log_destroy(logger);
	config_destroy(config);	
}