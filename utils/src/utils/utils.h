#ifndef UTILS_H_
#define UTILS_H_

//// INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <pthread.h>
#include <strings.h>

//// ESTRUCTURAS - ENUMERADORES

typedef enum {
	MENSAJE,
	PAQUETE,
  SOLICITUD_MEMORIA,
  PETICION_IO,
  SOLICITUD_INSTRUCCION,
  INSTRUCCION,
  OP_READ,
  OP_WRITE 
  PCB,
  SOLICITUD_DUMP_MEMORY,
  DATOS_MEMORIA,
  SOLICITUD_MARCO,
  ESCRITURA,
  LECTURA
  FINALIZACION_IO
} op_code_comunicacion;

typedef enum {
  SYSCALL_INIT_PROC,
  SYSCALL_EXIT,
  SYSCALL_IO,
  SYSCALL_DUMP_MEMORY
} tipo_syscall;

typedef enum {
  MODULO_KERNEL = 100,
  MODULO_MEMORIA = 101,
  MODULO_CPU = 102,
  MODULO_IO = 103
} header_modulo;

typedef enum {
  CPU_DISPATCH,
  CPU_INTERRUPT
} tipo_conexion_cpu;

typedef struct {
	int size;
	void* stream;
} t_buffer;

typedef struct {
	op_code_comunicacion codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef enum {
	NOOP,
	WRITE,
	READ,
	GOTO,
	IO,
	INIT_PROC,
	DUMP_MEMORY,
	EXIT
} t_tipo_instruccion;

typedef enum {
  IMPRESORA,
  TECLADO,
  MOUSE,
  AURICULARES,
  PARLANTE
} nombre_dispositivo_io;

typedef struct {
  t_tipo_instruccion tipo;
  char* parametro1;
  char* parametro2;
} t_instruccion;

typedef enum {
  ESTADO_NEW,
  ESTADO_READY,
  ESTADO_EXEC,
  ESTADO_BLOCKED,
  ESTADO_SUSP_BLOCKED,
  ESTADO_SUSP_READY,
  ESTADO_EXIT,
  CANTIDAD_ESTADOS
} t_estado;

typedef struct {
  uint32_t pid; //identificador del proceso
  uint32_t pc; //contador de programa
  uint32_t me[CANTIDAD_ESTADOS]; //cantidad de veces en un estado
  uint32_t mt[CANTIDAD_ESTADOS]; //tiempo que permanecio en ese estado en milisegundos
  char* dispositivo_actual;
  uint32_t duracion_io;
} t_pcb;

//// VARIABLES GLOBALES
extern t_log* logger;
extern t_config* config;


//// LOGGER
t_log* iniciar_logger(char*, char*, t_log_level);
void leer_consola(t_log*);
void iterator(char*);


//// CONFIG
t_config* iniciar_config(char*);


//// PAQUETES - MENSAJES
t_paquete* crear_paquete(int);
void crear_buffer(t_paquete*);
void paquete(int);
void agregar_a_paquete(t_paquete*, void*, int);
void* serializar_paquete(t_paquete*, int);
void enviar_paquete(t_paquete*, int);
void eliminar_paquete(t_paquete*);
void* serializar_mensaje(char*, int);
void enviar_mensaje(char*, int);
t_list* recibir_paquete(int);
void* recibir_buffer(int*, int);
void recibir_mensaje(int);


//// LISTAS
void agregar_puertos_a_lista(int,t_config*,t_list*);


//// CONEXIONES
int iniciar_servidor(char*);
int esperar_cliente(int);
int recibir_operacion(int);
int crear_conexion(char*, char*, int);
int32_t enviar_handshake_desde(int, int);
void* conectar_puertos_a_servidor(void*);
int conectarse_a(int , int , t_config*);
int iniciar_conexion(void*);

//// FINALIZAR
void terminar_programa(int, t_log*, t_config*);

/// KERNEL-IO
nombre_dispositivo_io obtener_dispositivo_io(char*);

#endif /* UTILS_H_ */
