#include "common_memoria.h"
#include "atencion_cpu.h"
#include "atencion_kernel.h"


////// VARIABLES EXTERNAS

void* memoria;
uint32_t tamanio_memoria;
uint32_t tamanio_pagina;
uint32_t entradas_por_tabla;
uint32_t cantidad_niveles;
uint32_t retardo_memoria;
uint32_t retardo_swap;
char* path_swapfile;
char* path_dump;
char* path_instrucciones;
uint32_t cantidad_marcos;
char* log_level;
uint32_t marcos_libres;
uint8_t* bitmap_marcos;
FILE* swapfile;
uint32_t swap_offset;
pthread_mutex_t mutex_memoria = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_swapfile = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_marcos_libres = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_bitmap = PTHREAD_MUTEX_INITIALIZER;

int servidor_memoria;
char* puerto_escucha;

t_list* lista_procesos;
t_list* lista_ids_cpus;

char* NOMBRES_INSTRUCCIONES[] = {
  "NOOP",
  "WRITE",
  "READ",
  "GOTO",
  "IO",
  "INIT_PROC",
  "DUMP_MEMORY",
  "EXIT"
};



////// INICIAR MEMORIA

void inicializar_memoria(void)
{
  config = iniciar_config("memoria.config");
  obtener_configs();

  t_log_level nivel_log = parse_log_level(log_level);

  logger = iniciar_logger("memoria.log", "Memoria", nivel_log);
  log_debug(logger, "Memoria iniciada!");


  memoria = malloc(tamanio_memoria);
  memset(memoria, 0, tamanio_memoria);

  cantidad_marcos = tamanio_memoria / tamanio_pagina;
  marcos_libres = cantidad_marcos;

  bitmap_marcos = malloc(cantidad_marcos * sizeof(uint8_t));
  memset(bitmap_marcos, 0, cantidad_marcos);

  pthread_mutex_init(&mutex_memoria, NULL);
  pthread_mutex_init(&mutex_swapfile, NULL);
  pthread_mutex_init(&mutex_marcos_libres, NULL);
  pthread_mutex_init(&mutex_bitmap, NULL);

  swapfile  = fopen(path_swapfile, "w+b");
  if (swapfile == NULL)
    log_error(logger, "Error: no se pudo crear el archivo SWAP");
  swap_offset = 0;


  servidor_memoria = iniciar_servidor(puerto_escucha);

  lista_procesos = list_create();
  lista_ids_cpus = list_create();
}

void obtener_configs(void)
{
  puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
  tamanio_memoria = config_get_int_value(config, "TAM_MEMORIA");
  tamanio_pagina = config_get_int_value(config, "TAM_PAGINA");
  entradas_por_tabla = config_get_int_value(config, "ENTRADAS_POR_TABLA");
  cantidad_niveles = config_get_int_value(config, "CANTIDAD_NIVELES");
  retardo_memoria = config_get_int_value(config, "RETARDO_MEMORIA");
  retardo_swap = config_get_int_value(config, "RETARDO_SWAP");
  path_swapfile = config_get_string_value(config, "PATH_SWAPFILE");
  path_dump = config_get_string_value(config, "DUMP_PATH");
  path_instrucciones = config_get_string_value(config, "PATH_INSTRUCCIONES");
  log_level = config_get_string_value(config, "LOG_LEVEL");
}



////// PROCESOS

t_proceso* obtener_proceso(uint32_t pid)
{
  if (list_is_empty(lista_procesos)) {
    log_warning(logger, "Proceso con PID: %d no encontrado", pid);
    return NULL;
  }
  for (int i = 0; i < list_size(lista_procesos); i++) 
  {
    t_proceso* proceso = list_get(lista_procesos, i);

    if (proceso->pid == pid) {
      return proceso;
    }
  }
  log_warning(logger, "Proceso con PID: %d no encontrado", pid);
  return NULL;
}



////// PAGINACION JERARQUICA MULTINIVEL

t_tabla* crear_tablas_multinivel(uint32_t nivel_actual, uint32_t* marcos_restantes) {
  if (*marcos_restantes == 0) return NULL;

  t_tabla* tabla = malloc(sizeof(t_tabla));
  tabla->nivel = nivel_actual;
  tabla->entradas = list_create();

  for (int index_entrada = 0; index_entrada < entradas_por_tabla; index_entrada++) {
    if (*marcos_restantes == 0) break;

    t_entrada* entrada = malloc(sizeof(t_entrada));

    if (nivel_actual < cantidad_niveles) {
      entrada->marco = -1;
      entrada->siguiente_tabla = crear_tablas_multinivel(nivel_actual + 1, marcos_restantes);
    } else {
      if (*marcos_restantes > 0) {
        entrada->marco = asignar_marco_libre(); 
        log_debug(logger, "Tabla nivel: %d; Entrada: %d; Marco asignado: %d",nivel_actual, index_entrada, entrada->marco);
        if (entrada->marco == -1) {
          log_warning(logger,"No se encontro ningun marco libre");
        } 
        entrada->siguiente_tabla = NULL;
        (*marcos_restantes)--;  
      }
      else {
        entrada->marco = -1;
        entrada->siguiente_tabla = NULL;
      }
    }

    list_add(tabla->entradas, entrada);
  }

  return tabla;
}

t_tabla* crear_tabla_multinivel(uint32_t* cantidad_marcos_proceso) 
{
  return crear_tablas_multinivel(1, cantidad_marcos_proceso);
}

uint32_t asignar_marco_libre(void)
{
  pthread_mutex_lock(&mutex_bitmap);
  for(int index_marco = 0; index_marco < cantidad_marcos; index_marco++)
  {
    if (bitmap_marcos[index_marco] == 0) {
      bitmap_marcos[index_marco] = 1;
      pthread_mutex_unlock(&mutex_bitmap);
      return index_marco;
    }
  }
  pthread_mutex_unlock(&mutex_bitmap);
  return -1;
}

void liberar_marcos(t_tabla* tabla_de_paginas) 
{
  log_warning(logger, "Funcion liberar marcos");
  if (tabla_de_paginas == NULL) return;

  if (tabla_de_paginas->entradas == NULL) return;

  if(list_is_empty(tabla_de_paginas->entradas)) {
    log_error(logger, "Error al liberar marcos");
    return;
  }
    
  for (int index_entrada = 0; index_entrada < list_size(tabla_de_paginas->entradas); index_entrada++) {
    t_entrada* entrada = list_get(tabla_de_paginas->entradas, index_entrada);

    if (entrada == NULL) {
      log_error(logger, "Error al liberar marcos");
      return;
    }

    if (entrada->siguiente_tabla != NULL) {
      liberar_marcos(entrada->siguiente_tabla);
    }

    else if (entrada->marco != -1) {
      if (entrada->marco > cantidad_marcos || entrada->marco < 0) {
        log_error(logger, "Error al liberar marcos");
        free(entrada);
        return;
      }
      pthread_mutex_lock(&mutex_bitmap);
      bitmap_marcos[entrada->marco] = 0;
      pthread_mutex_unlock(&mutex_bitmap);

      pthread_mutex_lock(&mutex_memoria);
      memset(memoria + entrada->marco * tamanio_pagina, 0, tamanio_pagina);
      pthread_mutex_unlock(&mutex_memoria);
    }

    free(entrada);
  }

  list_destroy(tabla_de_paginas->entradas);
  free(tabla_de_paginas);
}



////// ATENCION CLIENTES

void* atender_cliente(void* arg)
{
  int cliente_memoria = *(int*)arg;

  pthread_t hilo_atender;
  int32_t cliente = recibir_handshake_memoria(cliente_memoria);

  if (cliente == MODULO_KERNEL) {
    log_info(logger, "## Kernel Conectado - FD del socket: <%d>",cliente_memoria);
    pthread_create(&hilo_atender, NULL, atender_kernel, arg);
    pthread_detach(hilo_atender);
  }
  else if (cliente == MODULO_CPU) {
    pthread_create(&hilo_atender, NULL, atender_cpu, arg);
    pthread_join(hilo_atender,NULL);
  }
  else {
    log_warning(logger, "Error en el Handshake");
  }
  pthread_detach(pthread_self());
  return NULL;
};

int recibir_handshake_memoria(int cliente_memoria)
{
  int32_t handshake;
  int32_t resultado_ok = 0;
  int32_t resultado_error = -1;
  
  if (recv(cliente_memoria, &handshake, sizeof(int32_t), MSG_WAITALL) <= 0) {
      log_error(logger, "Error recibiendo handshake. Cerrando conexión...");
      close(cliente_memoria);
      return -1;
  }

  switch (handshake)
  {
  case MODULO_KERNEL:

    log_debug(logger, "Kernel conectado.");
    send(cliente_memoria, &resultado_ok, sizeof(int32_t), 0);
    return MODULO_KERNEL;
    break;

  case MODULO_CPU:
    int32_t identificador_cpu;
    recv(cliente_memoria,&identificador_cpu,sizeof(int32_t),MSG_WAITALL);
    bool misma_cpu(void* elem)
    {
      return ((t_cpu_id_socket*)elem)->id == identificador_cpu;
    }
    t_cpu_id_socket* nueva_cpu = list_find(lista_ids_cpus, misma_cpu);

    if (nueva_cpu == NULL) {
      nueva_cpu = malloc(sizeof(t_cpu_id_socket));
      nueva_cpu->id = identificador_cpu;
      nueva_cpu->socket = cliente_memoria;
      list_add(lista_ids_cpus, nueva_cpu);
      
      log_debug(logger, "CPU %d conectada.", identificador_cpu);
      send(cliente_memoria, &resultado_ok, sizeof(int32_t), 0);
      return MODULO_CPU;

    } else {
      log_error(logger,"Error, CPU ID: %d, ya esta conectada.", identificador_cpu);
      send(cliente_memoria, &resultado_error, sizeof(int32_t), 0);
      return -1;
    }
    break;

  default:
    send(cliente_memoria, &resultado_error, sizeof(int32_t), 0);
    break;
  }
  return -1;
}



///// FINALIZAR

void terminar_memoria(void)
{
  close(servidor_memoria);
  list_destroy_and_destroy_elements(lista_procesos, free);
  list_destroy_and_destroy_elements(lista_ids_cpus, free);
  pthread_mutex_destroy(&mutex_memoria);
  pthread_mutex_destroy(&mutex_swapfile);
  pthread_mutex_destroy(&mutex_marcos_libres);
  pthread_mutex_destroy(&mutex_bitmap);
	log_destroy(logger);
	config_destroy(config);	
}