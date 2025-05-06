#include "common_memoria.h"

uint32_t tamanio_memoria;
void* memoria;
uint32_t memoria_usada;
char* puerto_escucha;
int conexion_kernel;
//int conexion_cpu;

void inicializar_memoria(void)
{
  config = iniciar_config("memoria.config");
  logger = iniciar_logger("memoria.log", "Memoria", LOG_LEVEL_DEBUG);
  log_info(logger, "Log de Memoria iniciado");

  puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
  tamanio_memoria = config_get_int_value(config, "TAM_MEMORIA");

  memoria = malloc(tamanio_memoria);
  memoria_usada = 0;
}

void* conectar_kernel(void*) 
{
  conexion_kernel = iniciar_conexion(puerto_escucha);
  if (conexion_kernel == -1)
  pthread_exit(NULL);

  pthread_t hilo_atender_kernel;
  pthread_create(&hilo_atender_kernel, NULL, atender_kernel, NULL);
  pthread_join(hilo_atender_kernel,NULL);

  return NULL;
}

void* atender_kernel(void*)
{
  if (recibir_handshake_de(KERNEL, conexion_kernel) == -1) {
    pthread_exit((void*)EXIT_FAILURE);
  } else {

    t_list* lista;

    while (1) {
      int cod_op = recibir_operacion(conexion_kernel);
      switch (cod_op) {
      case PAQUETE:
        lista = recibir_paquete(conexion_kernel);
        list_iterate(lista, (void*) iterator);
        break;

      case SOLICITUD_MEMORIA:
        uint32_t tamanio_proceso;
        int32_t resultado_ok = 0;
        int32_t resultado_error = -1;

        recv(conexion_kernel,&tamanio_proceso,sizeof(int),MSG_WAITALL);
        if (verificar_espacio_memoria(tamanio_proceso))
          send(conexion_kernel,&resultado_ok,sizeof(int32_t),0);
        else
          send(conexion_kernel,&resultado_error,sizeof(int32_t),0);
        break;

      case UBICAR_PROCESO:
        void* ubicacion_proceso = recibir_y_ubicar_proceso();
        log_debug(logger,"Ubicacion del proceso en memoria: %p", ubicacion_proceso);

      case -1:
        log_error(logger, "El cliente se desconectó. Terminando servidor...");
        pthread_exit((void*)EXIT_FAILURE);
      default:
        log_warning(logger,"Operación desconocida. No quieras meter la pata.");
        break;
      }
    }
    return NULL;
  }
}

bool verificar_espacio_memoria(uint32_t tamanio_proceso)
{
  if(memoria_usada + tamanio_proceso > tamanio_memoria)
    return false;
  
  return true;
}

t_list* leer_archivo_instrucciones(char* path)
{
  FILE *file = fopen(path, "r");
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

void* ubicar_proceso_en_memoria(int tamanio_proceso, t_list* lista_instrucciones)
{ 
  int tamanio_lista = list_size(lista_instrucciones);

  if (tamanio_lista == 0) {
    log_error(logger, "Error: lista de instrucciones vacia.\n");
    return NULL;
  }

  void* ubicacion_proceso = malloc(tamanio_proceso);
  if (ubicacion_proceso == NULL) {
      log_error(logger, "Error al asignar memoria para el proceso.\n");
      return NULL; 
  }

  t_instruccion* instruccion;
  uint32_t tamanio_instruccion = sizeof(instruccion->tipo) + sizeof(instruccion->parametro1) + sizeof(instruccion->parametro2);

  for(int i = 0; i < tamanio_lista; i+=1) {
    instruccion = list_get(lista_instrucciones, i);
    log_debug(logger,"Instruccion: %d, Parametro: %d", instruccion->tipo,instruccion->parametro1);
    memcpy(ubicacion_proceso + i * tamanio_instruccion, instruccion, tamanio_instruccion); 
  }

  return ubicacion_proceso;
}

void* recibir_y_ubicar_proceso(void)
{
  t_list* valores = recibir_paquete(conexion_kernel);

  uint32_t longitud_path = *(int32_t*)list_get(valores, 0); // Primero necesitados la longitud del path para poder alojarlo en memoria
  char* path = malloc(longitud_path);
  memcpy(path, list_get(valores, 1), longitud_path); //ahora si guardamos el path en la variable path

  uint32_t tamanio_proceso = *(uint32_t*)list_get(valores, 2); // y aca se guarda el tamaño del proceso

  log_debug(logger,"Path recibido: %s",path);
  
  t_list* lista_instrucciones = leer_archivo_instrucciones(path);
  void* ubicacion_proceso = ubicar_proceso_en_memoria(tamanio_proceso,lista_instrucciones);

  return ubicacion_proceso;
}

void terminar_memoria(void)
{
  liberar_conexion(conexion_kernel);
  //liberar_conexion(conexion_cpu);
	log_destroy(logger);
	config_destroy(config);	
}