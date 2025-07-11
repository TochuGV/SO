#include "atencion_kernel.h"


void* atender_kernel(void* arg)
{
  int cliente_kernel = *(int*)arg;


    int cod_op = recibir_operacion(cliente_kernel);
    uint32_t pid;
    t_list* valores;

    int32_t resultado_ok = 0;
    int32_t resultado_error = -1;
    
    switch (cod_op) {
    
    case SOLICITUD_MEMORIA:
      if (recibir_y_ubicar_proceso(cliente_kernel) == 0) {
        send(cliente_kernel,&resultado_ok,sizeof(int32_t),0);
      }
      else
        send(cliente_kernel,&resultado_error,sizeof(int32_t),0);
      break;
    
    case FINALIZAR_PROCESO:
      valores = recibir_paquete(cliente_kernel);
      pid = *(uint32_t*) list_get(valores, 0);

      if (finalizar_proceso(pid) == 0) {
        send(cliente_kernel,&resultado_ok,sizeof(int32_t),0);
      }
      else
        send(cliente_kernel,&resultado_error,sizeof(int32_t),0);
      
      list_destroy_and_destroy_elements(valores, free);
      break;
    
    case SOLICITUD_DUMP_MEMORY:
      valores = recibir_paquete(cliente_kernel);
      pid = *(uint32_t*) list_get(valores, 0);

      if (atender_dump_memory(pid) == 0) {
        send(cliente_kernel,&resultado_ok,sizeof(int32_t),0);
      }
      else
        send(cliente_kernel,&resultado_error,sizeof(int32_t),0);
      
      list_destroy_and_destroy_elements(valores, free);
      break;
    
    case SUSPENDER:
      valores = recibir_paquete(cliente_kernel);
      pid = *(uint32_t*) list_get(valores, 0);

      log_warning(logger, "Proceso %d entro al case suspender", pid);

      suspender_proceso(pid);

      list_destroy_and_destroy_elements(valores, free);
      break;
    
    case DESUSPENDER:
      valores = recibir_paquete(cliente_kernel);
      pid = *(uint32_t*) list_get(valores, 0);

      if (desuspender_proceso(pid) == 0) {
        send(cliente_kernel,&resultado_ok,sizeof(int32_t),0);
      }
      else
        send(cliente_kernel,&resultado_error,sizeof(int32_t),0);

      list_destroy_and_destroy_elements(valores, free);
      break;

    case -1:
      log_error(logger, "Error Kernel se desconectó de forma extraña. Terminando servidor...");
      pthread_exit((void*)EXIT_FAILURE);
    default:
      log_warning(logger,"Operación desconocida. No quieras meter la pata.");
      break;
    }

  log_info(logger, "Kernel Desconectado - FD del socket: <%d>", cliente_kernel);
  pthread_exit((void*)EXIT_FAILURE);
  
  return NULL;
}


int recibir_y_ubicar_proceso(int cliente_kernel)
{
  t_list* valores = recibir_paquete(cliente_kernel);

  uint32_t tamanio_proceso = *(uint32_t*)list_get(valores, 2);

  uint32_t cantidad_marcos_proceso = (tamanio_proceso + tamanio_pagina - 1) / tamanio_pagina;

  if (verificar_espacio_memoria(cantidad_marcos_proceso)) {

    uint32_t longitud_archivo_pseudocodigo = *(uint32_t*)list_get(valores, 0); 

    char* archivo_pseudocodigo = malloc(longitud_archivo_pseudocodigo);
    memcpy(archivo_pseudocodigo, list_get(valores, 1), longitud_archivo_pseudocodigo); 

    uint32_t pid = *(uint32_t*)list_get(valores, 3);

    t_proceso* proceso = malloc(sizeof(t_proceso));
    proceso->pid = pid;
    proceso->lista_instrucciones = leer_archivo_instrucciones(archivo_pseudocodigo);
    proceso->tabla_de_paginas = crear_tabla_multinivel(&cantidad_marcos_proceso);
    proceso->marcos_en_uso = cantidad_marcos_proceso;
    memset(proceso->metricas, 0, sizeof(proceso->metricas));
    proceso->base_swap = -1;
    proceso->tamanio_swap = 0;

    list_add(lista_procesos,proceso);

    log_info(logger, "## PID: <%d> - Proceso Creado - Tamaño: <%d>",pid,tamanio_proceso);
    
    free(archivo_pseudocodigo);
    list_destroy_and_destroy_elements(valores, free);
    return 0;
  };

  log_warning(logger,"No hay lugar en memoria para el proceso.");
  list_destroy_and_destroy_elements(valores, free);
  return -1;
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

    for(int indice_instruccion = 0; indice_instruccion < CANTIDAD_INSTRUCCIONES; indice_instruccion++) {
      if (strcmp(token, NOMBRES_INSTRUCCIONES[indice_instruccion]) == 0) {
        instruccion.tipo = indice_instruccion;
        break;
      }
    }

    char* param1 = strtok(NULL, " \n");
    char* param2 = strtok(NULL, " \n");

    if (param1 != NULL)
      instruccion.parametro1 = strdup(param1);
    else
      instruccion.parametro1 = strdup("");

    if (param2 != NULL)
      instruccion.parametro2 = strdup(param2);
    else
      instruccion.parametro2 = strdup("");
    
    t_instruccion* nueva_instruccion = malloc(sizeof(t_instruccion));
    *nueva_instruccion = instruccion;

    list_add(lista_instrucciones, nueva_instruccion);

  }
  fclose(file);
  return lista_instrucciones;
}

bool verificar_espacio_memoria(uint32_t cantidad_marcos_proceso)
{
  if (marcos_libres >= cantidad_marcos_proceso) {

    pthread_mutex_lock(&mutex_marcos_libres);
    marcos_libres = marcos_libres - cantidad_marcos_proceso;
    pthread_mutex_unlock(&mutex_marcos_libres);
    return true;
  }
  return false;
}


int finalizar_proceso(uint32_t pid) 
{

  for (int i = 0; i < list_size(lista_procesos); i++) 
  {
    t_proceso* proceso = list_get(lista_procesos, i);

    if (proceso->pid == pid) {

      log_info(logger,
      "## PID: <%d> - Proceso Destruido - Métricas - "
      "Acc.T.Pag: <%d>; Inst.Sol.: <%d>; SWAP: <%d>; "
      "Mem.Prin.: <%d>; Lec.Mem.: <%d>; Esc.Mem.: <%d>",
      pid,
      proceso->metricas[ACCESOS_TABLA_PAGINAS],
      proceso->metricas[INSTRUCCIONES_SOLICITADAS],
      proceso->metricas[BAJADAS_A_SWAP],
      proceso->metricas[SUBIDAS_A_MP],
      proceso->metricas[LECTURAS],
      proceso->metricas[ESCRITURAS]);

      liberar_marcos(proceso->tabla_de_paginas);
      list_destroy_and_destroy_elements(proceso->lista_instrucciones, free);
      list_remove(lista_procesos, i);
      free(proceso);
      return 0;
    }
  }
  return -1;
}

int atender_dump_memory(uint32_t pid)
{
  t_proceso* proceso = obtener_proceso(pid);

  if(!proceso)
    return -1;
  
  log_info(logger, "## PID: <%d> - Memory Dump solicitado", pid);

  char* timestamp = temporal_get_string_time("%d-%m-%y_%H-%M-%S");

  char path_archivo[256];
  snprintf(path_archivo, sizeof(path_archivo), "%s<%d>-<%s>.dmp", path_dump, pid, timestamp);
  FILE *file = fopen(path_archivo, "wb");

  free(timestamp);

  if (file == NULL) {
    log_error(logger, "Error: no se pudo crear el archivo DUMP");
    return -1;
  }

  uint32_t tamanio_dump = proceso->marcos_en_uso * tamanio_pagina;

  if (ftruncate(fileno(file), tamanio_dump) == -1) {
    log_error(logger, "Error: no se pudo truncar el archivo DUMP");
    fclose(file);
    return -1;
  }

  escribir_dump(proceso->tabla_de_paginas, file);
  fclose(file);
  return 0;
}

void escribir_dump(t_tabla* tabla_de_paginas, FILE* file) 
{
  if (tabla_de_paginas == NULL) return;
    
  for (int index_entrada = 0; index_entrada < list_size(tabla_de_paginas->entradas); index_entrada++) {
    t_entrada* entrada = list_get(tabla_de_paginas->entradas, index_entrada);

    if (entrada->siguiente_tabla != NULL) {
      escribir_dump(entrada->siguiente_tabla, file);
    }

    else if (entrada->marco != -1) {
      uint32_t offset = entrada->marco * tamanio_pagina;
      fwrite(memoria + offset, 1, tamanio_pagina, file);
    }
  }
}

void suspender_proceso(uint32_t pid)
{
  t_proceso* proceso = obtener_proceso(pid);

  if(!proceso) {
    log_error(logger, "Error al suspender proceso");
    return;
  }

  proceso->base_swap = swap_offset;

  atender_dump_memory(pid);
  escribir_en_swap(proceso->tabla_de_paginas);
  usleep(retardo_swap * 1000);
  proceso->metricas[BAJADAS_A_SWAP]++;
  log_warning(logger, "Proceso %d suspendido", pid);

  proceso->tamanio_swap = proceso->marcos_en_uso * tamanio_pagina;
  proceso->marcos_en_uso = 0;

  liberar_marcos(proceso->tabla_de_paginas);
  return ;
}

void escribir_en_swap(t_tabla* tabla_de_paginas)
{
  if (tabla_de_paginas == NULL) return;

  for (int index_entrada = 0; index_entrada < list_size(tabla_de_paginas->entradas); index_entrada++) {
    t_entrada* entrada = list_get(tabla_de_paginas->entradas, index_entrada);

    if (entrada->siguiente_tabla != NULL) {
      escribir_en_swap(entrada->siguiente_tabla);
    }
    else if (entrada->marco != -1) {
      uint32_t offset = entrada->marco * tamanio_pagina;

      pthread_mutex_lock(&mutex_swapfile);
      fseek(swapfile, swap_offset, SEEK_SET);
      fwrite(memoria + offset, 1, tamanio_pagina, swapfile);
      pthread_mutex_unlock(&mutex_swapfile);

      pthread_mutex_lock(&mutex_swap_offset);
      swap_offset += tamanio_pagina;
      pthread_mutex_unlock(&mutex_swap_offset);
    }
  }
}

int desuspender_proceso(uint32_t pid)
{
  t_proceso* proceso = obtener_proceso(pid);

  if(!proceso)
    return -1;
  
  uint32_t cantidad_marcos_proceso = proceso->tamanio_swap / tamanio_pagina;

  if (verificar_espacio_memoria(cantidad_marcos_proceso)) {
    proceso->tabla_de_paginas = crear_tabla_multinivel(&cantidad_marcos_proceso);
    int32_t offset_swap_actual = proceso->base_swap;
    leer_swap(proceso->tabla_de_paginas, &offset_swap_actual);
    usleep(retardo_swap * 1000);
    proceso->metricas[SUBIDAS_A_MP]++;
    proceso->base_swap = -1;
    proceso->tamanio_swap = 0;
    proceso->marcos_en_uso = cantidad_marcos_proceso;
    return 0;
  }
  return -1;
}

void leer_swap(t_tabla* tabla_de_paginas, int32_t* offset_swap_actual)
{
  if (tabla_de_paginas == NULL) return;

  for (int index_entrada = 0; index_entrada < list_size(tabla_de_paginas->entradas); index_entrada++) {
    t_entrada* entrada = list_get(tabla_de_paginas->entradas, index_entrada);

    if (entrada->siguiente_tabla != NULL) {
      leer_swap(entrada->siguiente_tabla, offset_swap_actual);
    }
    else if (entrada->marco != -1) {
      uint32_t marco_offset = entrada->marco * tamanio_pagina;

      pthread_mutex_lock(&mutex_swapfile);
      fseek(swapfile, *offset_swap_actual, SEEK_SET);
      fread(memoria + marco_offset, 1, tamanio_pagina, swapfile);
      pthread_mutex_unlock(&mutex_swapfile);

      *offset_swap_actual += tamanio_pagina;
    }
  }
}
