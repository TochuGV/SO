#include "main.h"

int main(int argc, char* argv[]) 
{
  logger = iniciar_logger("cpu.log", "CPU", LOG_LEVEL_INFO);
  config = iniciar_config("cpu.config");

   int32_t identificador_cpu = atoi(argv[1]);

  iniciar_cpu(identificador_cpu);

  //Hilos
  pthread_t hilo_kernel_dispatch;
  pthread_t hilo_kernel_interrupt;
  pthread_t hilo_memoria;

  pthread_create(&hilo_kernel_dispatch, NULL, conectar_dispatch, datos_dispatch);
  pthread_create(&hilo_kernel_interrupt, NULL, conectar_interrupt, datos_interrupt);
  pthread_create(&hilo_memoria, NULL, conectar_memoria, datos_memoria);

  pthread_join(hilo_kernel_dispatch, NULL);
  pthread_join(hilo_kernel_interrupt, NULL);
  pthread_join(hilo_memoria, NULL);

  bool cpu_disponible=true;

  while (1) {
    if (cpu_disponible) {
        //Paso 1: Recibir el PCB desde Kernel
        t_pcb* pcb = recibir_pcb(conexion_kernel_dispatch);
        if (pcb == NULL) {
          log_info(logger, "No se recibió ningún PCB");
          continue;
        }
        cpu_disponible = false;
        ciclo_de_instruccion(pcb, conexion_kernel_dispatch, conexion_kernel_interrupt, conexion_memoria);
        cpu_disponible = true;
    }
}

  return EXIT_SUCCESS;
}

