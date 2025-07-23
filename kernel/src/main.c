#include "main.h"

int main(int argc, char* argv[]){
  inicializar_kernel();
  iniciar_conexiones_constantes_entre_modulos();
  esperar_enter_para_planificar();
  crear_proceso_inicial(argv[1], atoi(argv[2]));
  iniciar_planificadores();
  unir_hilos();
  return EXIT_SUCCESS;
};