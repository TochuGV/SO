#include "atencion_kernel.h"

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

  return lista_instrucciones;
}

int recibir_y_ubicar_proceso(int cliente_kernel)
{
  t_list* valores = recibir_paquete(cliente_kernel);

  uint32_t tamanio_proceso = *(uint32_t*)list_get(valores, 2); 

  if (verificar_espacio_memoria(tamanio_proceso)) {

    uint32_t longitud_archivo_pseudocodigo = *(int32_t*)list_get(valores, 0); 
    char* archivo_pseudocodigo = malloc(longitud_archivo_pseudocodigo);
    memcpy(archivo_pseudocodigo, list_get(valores, 1), longitud_archivo_pseudocodigo); 


    uint32_t pid = *(uint32_t*)list_get(valores, 3);

    log_debug(logger,"Proceso %s con pid %d recibido",archivo_pseudocodigo,pid);

    t_proceso* proceso = malloc(sizeof(t_proceso));
    proceso->pid = pid;
    proceso->lista_instrucciones = leer_archivo_instrucciones(archivo_pseudocodigo);

    list_add(lista_procesos,proceso);

    log_info(logger, "## PID: <%d> - Proceso Creado - Tamaño: <%d>",pid,tamanio_proceso);

    return 0;
  };

  log_warning(logger,"No hay lugar en memoria para el proceso.");
  return -1;
}