#include "main.h"

int main(int argc, char* argv[]) 
{
  int32_t identificador_cpu = atoi(argv[1]);
  char archivo_config[64];
  snprintf(archivo_config, sizeof(archivo_config), "cpu%d.config", identificador_cpu);
  config = iniciar_config(archivo_config);

  char* log_level = config_get_string_value(config, "LOG_LEVEL");
  t_log_level nivel_log = parse_log_level(log_level);

  char archivo_log[64];
  snprintf(archivo_log, sizeof(archivo_log), "cpu%d.log", identificador_cpu);
  logger = iniciar_logger(archivo_log, "CPU", nivel_log);

  ip_kernel = config_get_string_value(config,"IP_KERNEL");
  ip_memoria = config_get_string_value(config, "IP_MEMORIA");

  puerto_kernel_dispatch = config_get_string_value(config, "PUERTO_KERNEL_DISPATCH");
  puerto_kernel_interrupt = config_get_string_value(config, "PUERTO_KERNEL_INTERRUPT");
  puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");

  t_cpu* cpu = iniciar_cpu(identificador_cpu);

  //Hilos
  pthread_t hilo_kernel_dispatch;
  pthread_t hilo_kernel_interrupt;
  pthread_t hilo_memoria;

  pthread_create(&hilo_kernel_dispatch, NULL, conectar_dispatch, cpu);
  pthread_create(&hilo_kernel_interrupt, NULL, conectar_interrupt, cpu);
  pthread_create(&hilo_memoria, NULL, conectar_memoria, cpu);

  pthread_join(hilo_kernel_dispatch, NULL);
  pthread_join(hilo_kernel_interrupt, NULL);
  pthread_join(hilo_memoria, NULL);

  bool cpu_disponible=true;

  while(1){   
    if(cpu_disponible){
        //Paso 1: Recibir el PCB desde Kernel
        t_pcb* pcb = recibir_pcb(cpu->conexion_kernel_dispatch);
        if (pcb == NULL) {
          log_debug(logger, "recv devolvió -1. Loop infinito evitado.");
          log_warning(logger, "Se perdió la conexión con Kernel. Terminando CPU...");
          break;
        };
        cpu_disponible = false;
        ciclo_de_instruccion(cpu,pcb, cpu->conexion_kernel_dispatch, cpu->conexion_kernel_interrupt, cpu->conexion_memoria);
        cpu_disponible = true;
    }
  }
  liberar_cpu (cpu);
  return EXIT_SUCCESS;
}
