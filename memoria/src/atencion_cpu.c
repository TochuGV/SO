#include "atencion_cpu.h"

void* atender_cpu(void* arg)
{
  int cliente_cpu = *(int*)arg;

    while (1) {
      int cod_op = recibir_operacion(cliente_cpu);
      //log_debug(logger,"CODOP: %d",cod_op);
      switch (cod_op) {
      
      case DATOS_MEMORIA:
        t_paquete* paquete_datos = crear_paquete(DATOS_MEMORIA);

        agregar_a_paquete(paquete_datos, &tamanio_pagina, sizeof(uint32_t));
        agregar_a_paquete(paquete_datos, &entradas_por_tabla, sizeof(uint32_t));
        agregar_a_paquete(paquete_datos, &cantidad_niveles, sizeof(uint32_t)); 

        enviar_paquete(paquete_datos, cliente_cpu);

        break;

      case SOLICITUD_INSTRUCCION:
        //log_debug(logger,"Entre al caso SOLICITUD INSTRUCCION");
        recibir_solicitud_instruccion(cliente_cpu);
        break;
      
      case SOLICITUD_MARCO:
        recibir_solicitud_marco(cliente_cpu);
        break;

      case ESCRITURA:
        // Escribir los datos que indique cpu en la direccion fisica que reciba de cpu
        break;

      case LECTURA:
        // Leer el tamaño de memoria indicado por cpu en la direccion fisica que reciba de cpu
        break;

      case -1:
        log_error(logger, "CPU se desconectó. Terminando servidor...");
        bool misma_cpu(void* elem)
        {
          return ((t_cpu_id_socket*)elem)->socket == cliente_cpu;
        }
        t_cpu_id_socket* cpu_eliminada = list_remove_by_condition(lista_ids_cpus, misma_cpu);
        free(cpu_eliminada);
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
  t_instruccion* instruccion;

  t_list* solicitud = recibir_paquete(cliente_cpu);

  uint32_t pid = *(uint32_t*)list_get(solicitud, 0);
  uint32_t pc = *(uint32_t*)list_get(solicitud, 1);

  t_proceso* proceso = obtener_proceso(pid);

  if (!proceso) {
    log_warning(logger,"Error al encontrar el proceso que CPU solicita.");
    return NULL;
  }

  if (pc >= list_size(proceso->lista_instrucciones)) {
    log_error(logger, "El PC %d excede la cantidad de instrucciones del proceso %d.", pc, pid);
    return NULL;
  }

  instruccion = list_get(proceso->lista_instrucciones,pc);

  t_tipo_instruccion tipo = instruccion->tipo;
  char* parametro1 = instruccion->parametro1;
  char* parametro2 = instruccion->parametro2;

  log_info(logger, "## PID: <%d> - Obtener instrucción: <%d> - Instrucción: <%s> <%s  %s>",pid,pc,NOMBRES_INSTRUCCIONES[tipo],parametro1,parametro2);

  proceso->metricas[INSTRUCCIONES_SOLICITADAS]++;

  t_paquete* paquete = crear_paquete(INSTRUCCION);

  uint32_t longitud_parametro1 = strlen(parametro1) + 1;
  uint32_t longitud_parametro2 = strlen(parametro2) + 1;

  agregar_a_paquete(paquete, &tipo, sizeof(tipo));
  agregar_a_paquete(paquete, &longitud_parametro1, sizeof(uint32_t));
  agregar_a_paquete(paquete, parametro1, longitud_parametro1);
  agregar_a_paquete(paquete, &longitud_parametro2, sizeof(uint32_t));
  agregar_a_paquete(paquete, parametro2, longitud_parametro2);

  enviar_paquete(paquete, cliente_cpu);

  //log_debug(logger,"Proceso: %d; Instruccion numero: %d enviada a CPU.",pid,pc);

  return NULL;
}

void recibir_solicitud_marco(int cliente_cpu)
{
  t_list* valores = recibir_paquete(cliente_cpu);

  uint32_t pid = *(uint32_t*)list_get(valores, 0);
  t_proceso* proceso = obtener_proceso(pid);

  t_tabla* tabla_actual = proceso->tabla_de_paginas;
  t_entrada* entrada = NULL;

  if (list_size(valores) - 1 != cantidad_niveles) {
    log_error(logger, "Error al recibir las entradas de nivel del proceso con PID: %d", pid);
    return;
  }

  for (int nivel = 1; nivel <= cantidad_niveles; nivel++)
  {
    uint32_t entrada_nivel = *(uint32_t*)list_get(valores, nivel);

    entrada = list_get(tabla_actual->entradas, entrada_nivel);

    if (!entrada) {
      log_error(logger, "Error ecorriendo la tabla de paginas del proceso con PID: %d", pid);
      return;
    }

    proceso->metricas[ACCESOS_TABLA_PAGINAS]++;
    usleep(retardo_memoria * 1000);

    if (nivel != cantidad_niveles){
      tabla_actual = entrada->siguiente_tabla;
      if (!tabla_actual) {
        log_error(logger, "Error ecorriendo la tabla de paginas del proceso con PID: %d", pid);
        return;
      }
    } 
  }

  uint32_t marco = entrada->marco;

  if (marco == -1)
    log_warning(logger, "El marco solicitado del proceso con PID: %d, no existe. Se envia el valor -1", pid);

  send(cliente_cpu, &marco, sizeof(uint32_t), 0);

  list_destroy_and_destroy_elements(valores, free);
}