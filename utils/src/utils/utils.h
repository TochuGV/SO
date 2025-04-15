#ifndef UTILS_H_
#define UTILS_H_

//// INCLUDES
#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include<pthread.h>


//// ESTRUCTURAS - ENUMERADORES
typedef enum
{
	MENSAJE,
	PAQUETE,
  HANDSHAKE
}op_code;

typedef enum
{
	KERNEL,
  CPU,
  MEMORIA,
  IO
}header_modulo;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;


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
t_paquete* crear_paquete(void);
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
int recibir_handshake(int);
void* iniciar_conexion(void*);
void* conectar_puertos_a_servidor(void*);
int conectarse_a(int , int , t_config*);
void liberar_conexion(int);


//// FINALIZAR
void terminar_programa(int, t_log*, t_config*);



#endif /* UTILS_H_ */