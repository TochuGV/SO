#include "common_memoria.h"

uint32_t tamanio_memoria;
void* memoria;
uint32_t memoria_usada;
char* path_instrucciones;

int servidor_memoria;
char* puerto_escucha;

t_list* lista_procesos;
t_list* lista_ids_cpus;

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
  log_debug(logger, "Memoria iniciada!");

  puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
  tamanio_memoria = config_get_int_value(config, "TAM_MEMORIA");
  path_instrucciones = config_get_string_value(config, "PATH_INSTRUCCIONES");

  memoria = malloc(tamanio_memoria);
  memoria_usada = 0;

  servidor_memoria = iniciar_servidor(puerto_escucha);

  lista_procesos = list_create();
  lista_ids_cpus = list_create();
}

void* atender_cliente(void* arg)
{
  int cliente_memoria = *(int*)arg;
  pthread_t hilo_atender;
  int32_t cliente = recibir_handshake_memoria(cliente_memoria);

  if (cliente == KERNEL) {
    log_info(logger, "## Kernel Conectado - FD del socket: <%d>",cliente_memoria);
    pthread_create(&hilo_atender, NULL, atender_kernel, arg);
    pthread_detach(hilo_atender);
  }
  else if (cliente == CPU) {
    pthread_create(&hilo_atender, NULL, atender_cpu, arg);
    pthread_join(hilo_atender,NULL);
  }
  else {
    log_warning(logger, "Error en el Handshake");
  }
  pthread_detach(pthread_self());
  return NULL;

}

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
  case KERNEL:

    log_debug(logger, "Kernel conectado.");
    send(cliente_memoria, &resultado_ok, sizeof(int32_t), 0);
    return KERNEL;
    break;

  case CPU:
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
      return CPU;

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

void terminar_memoria(void)
{
  close(servidor_memoria);
  list_destroy_and_destroy_elements(lista_procesos, free);
  list_destroy_and_destroy_elements(lista_ids_cpus, free);
	log_destroy(logger);
	config_destroy(config);	
}