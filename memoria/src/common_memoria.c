#include "common_memoria.h"

uint32_t tamanio_memoria;
void* memoria;
uint32_t memoria_usada;
char* path_instrucciones;

int servidor_memoria;
char* puerto_escucha;

t_list* lista_procesos;

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
void* atender_cliente(void* arg)
{
  int cliente_memoria = *(int*)arg;
  pthread_t hilo_atender;
  int32_t cliente = recibir_handshake_memoria(cliente_memoria);

  if (cliente == KERNEL) {
    pthread_create(&hilo_atender, NULL, atender_kernel, arg);
    pthread_detach(hilo_atender);
  }
  else if (cliente == CPU) {
    pthread_create(&hilo_atender, NULL, atender_cpu, arg);
    pthread_join(hilo_atender,NULL);
  }
  else {
    log_warning(logger, "Handshake desconocido");
  }
  pthread_detach(pthread_self());
  return NULL;

}

int recibir_handshake_memoria(int cliente_memoria)
{
  int32_t handshake;
  int32_t resultado_ok = 0;
  int32_t resultado_error = -1;

  recv(cliente_memoria,&handshake,sizeof(int32_t),MSG_WAITALL);

  switch (handshake)
  {
  case KERNEL:
    send(cliente_memoria, &resultado_ok, sizeof(int32_t), 0);
    return KERNEL;
    break;

  case CPU:
    int32_t identificador_cpu;
    send(cliente_memoria, &resultado_ok, sizeof(int32_t), 0);
    recv(cliente_memoria,&identificador_cpu,sizeof(int32_t),MSG_WAITALL);
    log_debug(logger,"CPU %d conectada.",identificador_cpu);
    return CPU;
    break;
  
  default:
    send(cliente_memoria, &resultado_error, sizeof(int32_t), 0);
    break;
  }
  return -1;
}

void* atender_kernel(void* arg)
{
  int cliente_kernel = *(int*)arg;

    t_list* lista;

    while (1) {
      int cod_op = recibir_operacion(cliente_kernel);
      
      switch (cod_op) {
      case PAQUETE:
        lista = recibir_paquete(cliente_kernel);
        list_iterate(lista, (void*) iterator);
        break;

      case SOLICITUD_MEMORIA:

        int32_t resultado_ok = 0;
        int32_t resultado_error = -1;

        if (recibir_y_ubicar_proceso(cliente_kernel) == 0) {
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
      }
    }
  return NULL;
}

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
      send(cliente_cpu,&instruccion,sizeof(t_instruccion),0);

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
  if (file == NULL) {
    log_error(logger, "Error: no se pudo ingresar al proceso");
    return NULL;
  }

  t_list* lista_instrucciones = list_create();
  t_instruccion instruccion;
  char linea[256];

  while ((fgets(linea, sizeof(linea), file) != NULL)) {
    char* token = strtok(linea, " \n");

    if (strcmp(token, "NOOP") == 0)
      instruccion.tipo = NOOP;
    else if(strcmp(token, "WRITE") == 0)
      instruccion.tipo = WRITE;
    else if(strcmp(token, "READ") == 0)
      instruccion.tipo = READ;
    else if(strcmp(token, "GOTO") == 0)
      instruccion.tipo = GOTO;
    else if(strcmp(token, "INSTRUCCION_IO") == 0)
      instruccion.tipo = INSTRUCCION_IO;
    else if(strcmp(token, "INIT_PROC") == 0)
      instruccion.tipo = INIT_PROC;
    else if(strcmp(token, "DUMP_MEMORY") == 0)
      instruccion.tipo = DUMP_MEMORY;
    else if(strcmp(token, "EXIT") == 0)
      instruccion.tipo = EXIT;

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

  uint32_t tamanio_proceso = *(uint32_t*)list_get(valores, 2); 

  if (verificar_espacio_memoria(tamanio_proceso)) {

    uint32_t longitud_path = *(int32_t*)list_get(valores, 0); 
    char* path = malloc(longitud_path);
    memcpy(path, list_get(valores, 1), longitud_path); 


    uint32_t pid = *(uint32_t*)list_get(valores, 3);

    log_debug(logger,"Proceso %s con pid %d recibido",path,pid);

    t_proceso* proceso = malloc(sizeof(t_proceso));
    proceso->pid = pid;
    proceso->lista_instrucciones = leer_archivo_instrucciones(path);

    list_add(lista_procesos,proceso);

    return 0;
  }

  log_warning(logger,"No hay lugar en memoria para el proceso.");
  return -1;
}


void terminar_memoria(void)
{
  close(servidor_memoria);
	log_destroy(logger);
	config_destroy(config);	
}