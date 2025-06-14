#include "atencion_cpu.h"



void* atender_cpu(void* arg)
{
  int cliente_cpu = *(int*)arg;

    t_list* lista;
    while (1) {
      int cod_op = recibir_operacion(cliente_cpu);
      //log_debug(logger,"CODOP: %d",cod_op);
      switch (cod_op) {
      case PAQUETE:
        lista = recibir_paquete(cliente_cpu);
        list_iterate(lista, (void*) iterator);
        break;
      case SOLICITUD_INSTRUCCION:
        //log_debug(logger,"Entre al caso SOLICITUD INSTRUCCION");
        recibir_solicitud_instruccion(cliente_cpu);
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

