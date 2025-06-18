#include "atencion_kernel.h"



void* atender_kernel(void* arg)
{
  int cliente_kernel = *(int*)arg;

  while (1) {
    int cod_op = recibir_operacion(cliente_kernel);
    uint32_t pid;
    t_list* valores;

    switch (cod_op) {

    case SOLICITUD_MEMORIA:

      int32_t resultado_ok = 0;
      int32_t resultado_error = -1;

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
      break;
    
    case SOLICITUD_DUMP_MEMORY:
      valores = recibir_paquete(cliente_kernel);
      pid = *(uint32_t*) list_get(valores, 0);

      if (atender_dump_memory(pid) == 0) {
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

    //log_debug(logger, "Tipo: %d", instruccion.tipo);
    //log_debug(logger, "Parametro 1: %s", instruccion.parametro1);
    //log_debug(logger, "Parametro 2: %s", instruccion.parametro2);
  }
  fclose(file);
  return lista_instrucciones;
}

bool verificar_espacio_memoria(uint32_t cantidad_marcos_proceso)
{
  if (marcos_libres >= cantidad_marcos_proceso) {
    marcos_libres = marcos_libres - cantidad_marcos_proceso;
    return true;
  }
  return false;
}

int recibir_y_ubicar_proceso(int cliente_kernel)
{
  t_list* valores = recibir_paquete(cliente_kernel);

  uint32_t tamanio_proceso = *(uint32_t*)list_get(valores, 2);

  uint32_t cantidad_marcos_proceso = (tamanio_proceso + tamanio_pagina - 1) / tamanio_pagina;

  if (verificar_espacio_memoria(cantidad_marcos_proceso)) {

    uint32_t longitud_archivo_pseudocodigo = *(int32_t*)list_get(valores, 0); 

    char* archivo_pseudocodigo = malloc(longitud_archivo_pseudocodigo);
    memcpy(archivo_pseudocodigo, list_get(valores, 1), longitud_archivo_pseudocodigo); 

    uint32_t pid = *(uint32_t*)list_get(valores, 3);

    //log_debug(logger,"Proceso %s con pid %d recibido",archivo_pseudocodigo,pid);

    t_proceso* proceso = malloc(sizeof(t_proceso));
    proceso->pid = pid;
    proceso->lista_instrucciones = leer_archivo_instrucciones(archivo_pseudocodigo);
    proceso->tabla_de_paginas = crear_tabla_multinivel(&cantidad_marcos_proceso);
    proceso->marcos_en_uso = cantidad_marcos_proceso;
    memset(proceso->metricas, 0, sizeof(proceso->metricas));

    list_add(lista_procesos,proceso);

    log_info(logger, "## PID: <%d> - Proceso Creado - Tamaño: <%d>",pid,tamanio_proceso);

    return 0;
  };

  log_warning(logger,"No hay lugar en memoria para el proceso.");
  return -1;
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

  time_t timestamp = time(NULL);
  char path_archivo[256];
  snprintf(path_archivo, sizeof(path_archivo), "%s<%d>-<%ld>.dmp", path_dump, pid, (long)timestamp);
  FILE *file = fopen(path_archivo, "wb");

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
