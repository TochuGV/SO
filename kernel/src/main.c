#include "main.h"

int main(int argc, char* argv[]){
  inicializar_kernel();
  iniciar_conexiones_constantes_entre_modulos();
  esperar_enter_para_planificar();
  crear_proceso_inicial(argv[1], atoi(argv[2]));
  iniciar_planificadores();
  unir_hilos();

  //terminar_programa(conexion_memoria, logger, config); --> 'conexion_memoria' ya no existe más.
  dictionary_destroy_and_destroy_elements(diccionario_contextos_io, destruir_contexto_io);

  //destruimos el diccionario de estimaciones que se usa para sjf
  //dictionary_destroy_and_destroy_elements(diccionario_estimaciones, free);

  //Se pueden destruir logs, configs, conexiones, listas con elementos, semáforos, diccionarios, etc.
	return EXIT_SUCCESS;
};