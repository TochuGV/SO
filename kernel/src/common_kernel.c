#include "common_kernel.h"

int conexion_cpu_dispatch;
int conexion_cpu_interrupt;
int conexion_io;

void* conectar_cpu_dispatch(void* arg) 
{
  char* puerto = (char*) arg;
  int socket_cpu_dispatch = iniciar_conexion(puerto);
  if (socket_cpu_dispatch == -1)
  pthread_exit(NULL);

  pthread_t hilo_atender_cpu_dispatch;
  pthread_create(&hilo_atender_cpu_dispatch, NULL, atender_cpu_dispatch, &socket_cpu_dispatch);
  pthread_join(hilo_atender_cpu_dispatch,NULL);

  return NULL;
}

void* conectar_io(void* arg) 
{
  char* puerto = (char*) arg;
  int socket_io = iniciar_conexion(puerto);
  if (socket_io == -1)
  pthread_exit(NULL);

  pthread_t hilo_atender_io;
  pthread_create(&hilo_atender_io, NULL, atender_io, &socket_io);
  pthread_join(hilo_atender_io,NULL);

  return NULL;
}

void* atender_cpu_dispatch(void* arg)
{
  int socket_cpu_dispatch = *(int*)arg;
  if (recibir_handshake_de(CPU,socket_cpu_dispatch) == -1) {
    pthread_exit((void*)EXIT_FAILURE);
  } else {

  t_list* lista;

    while (1) {
      int cod_op = recibir_operacion(socket_cpu_dispatch);
      switch (cod_op) {
      case PAQUETE:
        lista = recibir_paquete(socket_cpu_dispatch);
        list_iterate(lista, (void*) iterator);
        break;
      //case SYSCALL:
        //recibir_syscall();
        //break;
      case -1:
        log_error(logger, "El cliente se desconectó. Terminando servidor...");
        pthread_exit((void*)EXIT_FAILURE);
      default:
        log_warning(logger,"Operación desconocida. No quieras meter la pata.");
        break;
      }
    }
    return NULL;
  }
}

void* atender_io(void* arg)
{
  int socket_io = *(int*)arg;
  if (recibir_handshake_de(IO,socket_io) == -1) {
    pthread_exit((void*)EXIT_FAILURE);
  } else {
    
  t_list* lista;

    while (1) {
      int cod_op = recibir_operacion(socket_io);
      switch (cod_op) {
      case PAQUETE:
        lista = recibir_paquete(socket_io);
        list_iterate(lista, (void*) iterator);
        break;
      case -1:
        log_error(logger, "El cliente se desconectó. Terminando servidor...");
        pthread_exit((void*)EXIT_FAILURE);
      default:
        log_warning(logger,"Operación desconocida. No quieras meter la pata.");
        break;
      }
    }
    return NULL;
  }
}

int contadorPid = 0;
const uint32_t CANTIDAD_ESTADOS = 7;

t_pcb* crear_pcb(){
  t_pcb* pcb = malloc(sizeof(t_pcb));
  if(!pcb) return NULL;

  pcb->pid = contadorPid++;
  pcb->pc = 0;
  pcb->me = calloc(CANTIDAD_ESTADOS, sizeof(uint32_t));
  pcb->mt = calloc(CANTIDAD_ESTADOS, sizeof(uint32_t));

  return pcb;
};

void destruir_pcb(t_pcb* pcb){
  if(!pcb) return;

  free(pcb->me);
  free(pcb->mt);
  free(pcb);
}

void* serializar_pcb(t_pcb* pcb, int bytes){
  /*
  int tamanio_me = sizeof(uint32_t) * 7;
  int tamanio_mt = sizeof(uint32_t) * 7;
  int tamanio = sizeof(uint32_t) * 2 + tamanio_me + tamanio_mt;

  Se pueden calcular los bytes antes de llamar a la función, o calcularlos dentro de la función o crear otra.
  Si se calculan antes, el parámetro es 'int bytes', sino tendría que ser 'int* bytes'.
  */

  void* magic = malloc(bytes);
  int desplazamiento = 0;

  memcpy(magic + desplazamiento, &(pcb->pid), sizeof(uint32_t));
  desplazamiento += sizeof(uint32_t);
  memcpy(magic + desplazamiento, &(pcb->pc), sizeof(uint32_t));
  desplazamiento += sizeof(uint32_t);
  memcpy(magic + desplazamiento, &(pcb->me), sizeof(uint32_t) * 7);
  desplazamiento += sizeof(uint32_t) * 7;
  memcpy(magic + desplazamiento, &(pcb->mt), sizeof(uint32_t) * 7);
  desplazamiento += sizeof(uint32_t) * 7;

  return magic;
}

void* enviar_proceso_a_memoria(char* path, uint32_t tamanio_proceso, int socket_cliente)
{
  t_paquete* paquete = crear_paquete(PATH);
  uint32_t longitud_path = strlen(path) + 1;
  agregar_a_paquete(paquete, &longitud_path, sizeof(uint32_t));// Enviar la longitud
  agregar_a_paquete(paquete, path, longitud_path);            // Enviar el contenido del path
  agregar_a_paquete(paquete, &tamanio_proceso, sizeof(tamanio_proceso));

  enviar_paquete(paquete, socket_cliente);

  log_info(logger,"Path enviado a memoria!");

  return NULL;
}

int32_t handshake_kernel(int conexion_memoria)
{
  int32_t handshake = KERNEL;
  int32_t resultado;

  send(conexion_memoria, &handshake, sizeof(int32_t), 0);
  recv(conexion_memoria, &resultado, sizeof(int32_t), MSG_WAITALL);
  if (resultado == -1) {
    log_error(logger, "Error: La conexión con Memoria falló. Finalizando conexión...");
    return -1;
  }
  return 0;
}