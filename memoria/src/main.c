#include "main.h"

int main(int argc, char* argv[]) 
{
  inicializar_memoria();
	
  pthread_t hilo_conexion_kernel;
  //pthread_t hilo_conexion_cpu;
  
  pthread_create(&hilo_conexion_kernel, NULL, conectar_kernel, NULL);
  //pthread_create(&hilo_conexion_cpu, NULL, conectar_cpu, puerto_escucha);

  pthread_join(hilo_conexion_kernel,NULL);
  //pthread_join(hilo_conexion_cpu,NULL);

  terminar_memoria();

  return EXIT_SUCCESS;

}

