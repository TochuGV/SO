#include "utils.h"


//// VARIABLES GLOBALES
t_log* logger;
t_config* config;


//// LOGGER
t_log* iniciar_logger(char* file_name, char* process_name, t_log_level level)
{
	t_log* nuevo_logger;
	nuevo_logger = log_create(file_name, process_name, true, level);
	if (nuevo_logger == NULL){
		perror("Error. No se pudo crear o encontrar el archivo.");
		abort();
	};
	return nuevo_logger;
}

void iterator(char* value) {
	log_info(logger, "%s", value);
}

void leer_consola(t_log* logger)
{
	char* leido = NULL;

	leido = readline("> ");
	log_info(logger, ">> %s", leido);

	while(strcmp(leido, "exit") != 0){
        free(leido);
        leido = readline("> ");
		log_info(logger, ">> %s", leido);
    };

	free(leido);
}


//// CONFIG
t_config* iniciar_config(char* config_name)
{
	t_config* nuevo_config;
	nuevo_config = config_create(config_name);
	if(nuevo_config == NULL){
		perror("No se pudo crear la config");
		abort();
	};
	return nuevo_config;
}


//// PAQUETES - MENSAJES
t_paquete* crear_paquete(void)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;
}

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

void paquete(int conexion)
{
	char* leido = NULL;
	t_paquete* paquete;
	leido = readline("> ");

	while(strcmp(leido, "exit") != 0){
    paquete = crear_paquete();
		agregar_a_paquete(paquete, leido, strlen(leido) + 1);
    enviar_paquete(paquete, conexion);
    free(leido);
    leido = readline("> ");
  }
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);
  eliminar_paquete(paquete);
	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
} 

void* serializar_mensaje(char* mensaje, int bytes)
{
  void * magic = malloc(bytes);
  int desplazamiento = 0;
  
  memcpy(magic + desplazamiento, &(mensaje), sizeof(int));
  desplazamiento+= sizeof(int);
  return magic;
}

void enviar_mensaje(char* mensaje, int socket_cliente)
{
	
  t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);
	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "%s", buffer);
	free(buffer);
}


//// LISTAS
void agregar_puertos_a_lista(int header, t_config* config, t_list* lista)
{
  switch (header)
  {
    case KERNEL:
      char* puerto_io = config_get_string_value(config, "PUERTO_ESCUCHA_IO");
	    char* puerto_cpu_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
      //char* puerto_cpu_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT"););
      list_add_in_index(lista, 0, puerto_io);
      list_add_in_index(lista, 1, puerto_cpu_dispatch);
      //list_add_index(lista, 2, puerto_cpu_interrupt);
      break;
    case CPU:
      char* puerto_cpu_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");
      list_add_in_index(lista, 0, puerto_cpu_interrupt);
      break;
    case MEMORIA:
      char* puerto_memoria = config_get_string_value(config, "PUERTO_ESCUCHA");
      list_add_in_index(lista, 0, puerto_memoria);
      break;
    case IO:
      break;
    default:
      break;
  }
}


//// CONEXIONES
int iniciar_servidor(char* puerto)
{
	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &servinfo);

	int socket_servidor = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

	setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));

	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);
	log_trace(logger, "Listo para escuchar a mi cliente");

	return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	//log_info(logger, "Se conecto un cliente!");

	return socket_cliente;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

int crear_conexion(char *ip, char* puerto, int header_cliente)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

  // Como el kernel es el unico que es cliente y servidor a la vez, no esta bueno
  // que se quede esperando al servidor siempre que se ejecuta, sino si se lo requiere
  // solo como servidor se va a quedar trabado y no va a finalizar
  
  if (header_cliente == KERNEL) {
    if (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1) {
      log_warning(logger, "No se pudo conectar a Memoria. El Kernel seguirá solo como servidor.");
      freeaddrinfo(server_info);
      return -1;
    }
  } 

  // Como IO y CPU son solo clientes si es correcto que esperen a la presencia de un servidor
  else {
    log_info(logger, "Esperando servidor...");
	  while (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1) {
    sleep(1);
  }
  }

	freeaddrinfo(server_info);

	return socket_cliente;
}

int32_t enviar_handshake_desde(int header_cliente, int socket_cliente)
{
  int32_t handshake;
  int32_t resultado;
  switch (header_cliente)
  {
  case KERNEL:
    handshake = 1;
    send(socket_cliente, &handshake, sizeof(int32_t), 0);
    recv(socket_cliente, &resultado, sizeof(int32_t), MSG_WAITALL);
    break;
  case CPU:
    handshake = 2;
    send(socket_cliente, &handshake, sizeof(int32_t), 0);
    recv(socket_cliente, &resultado, sizeof(int32_t), MSG_WAITALL);
    break;
  case IO:
    handshake = 3;
    send(socket_cliente, &handshake, sizeof(int32_t), 0);
    recv(socket_cliente, &resultado, sizeof(int32_t), MSG_WAITALL);
    break;
  
  default:
    break;
  }
  return resultado;
}

int recibir_handshake(int socket_cliente)
{
  int32_t handshake;
  int32_t resultado_ok = 0;
  int32_t resultado_error = -1;

  recv(socket_cliente, &handshake, sizeof(int32_t), MSG_WAITALL);

  switch (handshake)
  {
  case 1:
    send(socket_cliente, &resultado_ok, sizeof(int32_t), 0);
    log_info(logger, "El modulo Kernel se ah conectado exitosamente!");
    break;
  case 2:
    send(socket_cliente, &resultado_ok, sizeof(int32_t), 0);
    log_info(logger, "El modulo CPU se ah conectado exitosamente!");
    break;
  case 3:
    send(socket_cliente, &resultado_ok, sizeof(int32_t), 0);
    log_info(logger, "El modulo IO se ah conectado exitosamente!");
    break;
  
  default:
    send(socket_cliente, &resultado_error, sizeof(int32_t), 0);
    log_error(logger, "Error al conectar: cliente desconocido");
    return 0;
  }
  return 1;
}

void* iniciar_conexion(void* arg) 
{
  char* puerto = (char*)arg;

	int socket_servidor = iniciar_servidor(puerto);
	log_info(logger, "Listo para recibir al cliente");
	int socket_cliente = esperar_cliente(socket_servidor);

  if (recibir_handshake(socket_cliente) == 0) {
    pthread_exit((void*)EXIT_FAILURE);
  } else {

    t_list* lista;

    while (1) {
      int cod_op = recibir_operacion(socket_cliente);
      switch (cod_op) {
      case MENSAJE:
        recibir_mensaje(socket_cliente);
        break;
      case PAQUETE:
        lista = recibir_paquete(socket_cliente);
        list_iterate(lista, (void*) iterator);
        break;
      case -1:
        log_error(logger, "El cliente se desconecto. Terminando servidor");
        pthread_exit((void*)EXIT_FAILURE);
      default:
        log_warning(logger,"Operacion desconocida. No quieras meter la pata");
        break;
      }
    }
    return NULL;
  }
}

void* conectar_puertos_a_servidor(void* arg)
{
  t_list* lista_puertos = (t_list*)arg;
  int lista_tamanio = list_size(lista_puertos);
  pthread_t thread_puerto[lista_tamanio];
  char* puerto[lista_tamanio];
  int i = 0;
  while(i < lista_tamanio)
  {
    puerto[i] = list_get(lista_puertos, i);
    pthread_create(&thread_puerto[i], NULL, iniciar_conexion, puerto[i]);
    i+=1;
  }
  i = 0;
  while(i < lista_tamanio)
  {
    pthread_join(thread_puerto[i],NULL);
    i+=1;
  }
  return NULL;
}

int conectarse_a(int header_servidor, int header_cliente, t_config* config)
{
  char* ip;
  char* puerto;
  int conexion;

  // Elegir el cliente
  switch (header_cliente)
  {



    case KERNEL:
      // Para este cliente elegir el servidor
      switch (header_servidor)
      {
      case MEMORIA:
        ip = config_get_string_value(config, "IP_MEMORIA");
        puerto = config_get_string_value(config, "PUERTO_MEMORIA");
        conexion = crear_conexion(ip, puerto, header_cliente);
        if (conexion == -1) {
          break;
        } else {
          if (enviar_handshake_desde(KERNEL, conexion) == 0) {
          log_info(logger, "Conexion con modulo Memoria exitosa!");
          paquete(conexion);
          break;
          } else {
          log_error(logger, "Error: la conexion con el servidor fallo");
          }
        }
        
        
      default:
        break;
      }
      break;
      


    case CPU:
      // Para este cliente elegir el servidor
      switch (header_servidor)
      {
      case KERNEL:
        ip = config_get_string_value(config, "IP_KERNEL");
        puerto = config_get_string_value(config, "PUERTO_KERNEL_DISPATCH");
        conexion = crear_conexion(ip, puerto, header_cliente);
        if (enviar_handshake_desde(CPU, conexion) == 0) {
          log_info(logger, "Conexion con modulo Kernel exitosa!");
          paquete(conexion);
          break;
        } else {
          log_error(logger, "Error: la conexion con el servidor fallo");
        }
        break;
      case MEMORIA:
        ip = config_get_string_value(config, "IP_MEMORIA");
        puerto = config_get_string_value(config, "PUERTO_MEMORIA");
        conexion = crear_conexion(ip, puerto, header_cliente);
        if (enviar_handshake_desde(CPU, conexion) == 0) {
          log_info(logger, "Conexion con modulo Memoria exitosa!");
          paquete(conexion);
          break;
        } else {
          log_error(logger, "Error: la conexion con el servidor fallo");
        }
        break;
      default:
        break;
      }
      break;



    case IO:
      // Para este cliente elegir el servidor
      switch (header_servidor)
      {
      case KERNEL:
        ip = config_get_string_value(config, "IP_KERNEL");
        puerto = config_get_string_value(config, "PUERTO_KERNEL");
        conexion = crear_conexion(ip, puerto, header_cliente);
        if (enviar_handshake_desde(IO, conexion) == 0) {
          log_info(logger, "Conexion con modulo Kernel exitosa!");
          paquete(conexion);
          break;
        } else {
          log_error(logger, "Error: la conexion con el servidor fallo");
        }
        break;
      default:
        break;
      }
      break;



    default:
      break;

  }
  return conexion;
}


void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}


//// FINALIZAR
void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	liberar_conexion(conexion);
	log_destroy(logger);
	config_destroy(config);	
}

