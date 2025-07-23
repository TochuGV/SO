#include "utils.h"
#include <sys/time.h>

//// VARIABLES GLOBALES
t_log* logger;
t_config* config;

//// LOGGER
t_log* iniciar_logger(char* file_name, char* process_name, t_log_level level){
	t_log* nuevo_logger;
	nuevo_logger = log_create(file_name, process_name, true, level);
	if (nuevo_logger == NULL){
		perror("Error. No se pudo crear o encontrar el archivo.");
		abort();
	};
	return nuevo_logger;
}

void iterator(char* value){
	log_info(logger, "%s", value);
}

void leer_consola(t_log* logger){
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

t_log_level parse_log_level(const char* level_str) {
  if (strcmp(level_str, "TRACE") == 0) return LOG_LEVEL_TRACE;
  if (strcmp(level_str, "INFO") == 0) return LOG_LEVEL_INFO;
  if (strcmp(level_str, "WARNING") == 0) return LOG_LEVEL_WARNING;
  if (strcmp(level_str, "ERROR") == 0) return LOG_LEVEL_ERROR;
  if (strcmp(level_str, "DEBUG") == 0) return LOG_LEVEL_DEBUG;
  return LOG_LEVEL_INFO;
};


//// CONFIG
t_config* iniciar_config(char* config_name){
	t_config* nuevo_config;
	nuevo_config = config_create(config_name);
	if(nuevo_config == NULL){
		perror("No se pudo crear la config");
		abort();
	};
	return nuevo_config;
}


//// PAQUETES - MENSAJES
// Aca va el parametro de codop
t_paquete* crear_paquete(int cod_op){
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = cod_op;
	crear_buffer(paquete);
	return paquete;
}

void crear_buffer(t_paquete* paquete){
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

// Esta funcion se podria llamar "llenar el buffer del paquete"
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio){
  // En la direccion de memoria donde esta el stream del buffer haces
  // un espacio del tamaño que ocupe lo que estes empaquetando
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	if(valor != NULL && tamanio > 0){
    memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);
  };
	paquete->buffer->size += tamanio + sizeof(int); // Actualiza el tamaño del buffer
}

void enviar_paquete(t_paquete* paquete, int socket_cliente){
  // Tamaño del buffer y luego 2 veces int, 1 por la variable "int size",
  // y otra por la variable op_code, que es un int
	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);
	send(socket_cliente, a_enviar, bytes, 0);
  eliminar_paquete(paquete);
	free(a_enviar);
}

void* serializar_paquete(t_paquete* paquete, int bytes){
	void* a_enviar = malloc(bytes);
	int desplazamiento = 0;
	memcpy(a_enviar + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(a_enviar + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(a_enviar + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento += paquete->buffer->size;
	return a_enviar;
}

void eliminar_paquete(t_paquete* paquete){
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
} 

void* serializar_mensaje(char* mensaje, int bytes){ //esta funcion nose porq existe, no la usa nadie
  void * magic = malloc(bytes);
  int desplazamiento = 0;
  
  memcpy(magic + desplazamiento, &(mensaje), sizeof(int));
  desplazamiento+= sizeof(int);
  return magic;
}

void enviar_mensaje(char* mensaje, int socket_cliente){
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

t_list* recibir_paquete(int socket_cliente){
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size){ //ser recorre todo el buffer
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		void* valor = tamanio > 0 ? malloc(tamanio) : NULL;
    if(tamanio > 0)
      memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	};
	free(buffer);
	return valores;
}

void* recibir_buffer(int* size, int socket_cliente){
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente){
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "%s", buffer);
	free(buffer);
}

void paquete(int conexion){
	char* leido = NULL;
	t_paquete* paquete;
	leido = readline("> ");

	while(strcmp(leido, "exit") != 0){
    paquete = crear_paquete(PAQUETE);
		agregar_a_paquete(paquete, leido, strlen(leido) + 1);
    enviar_paquete(paquete, conexion);
    free(leido);
    leido = readline("> ");
  }
}

//// CONEXIONES
int iniciar_servidor(char* puerto){
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
  
  log_trace(logger, "Servidor iniciado.");
  
  return socket_servidor;
};

int esperar_cliente(int socket_servidor){
  int socket_cliente = accept(socket_servidor, NULL, NULL);
  return socket_cliente;
};

int recibir_operacion(int socket_cliente){
	int cod_op;
  int r = recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL);
  //log_debug(logger, "recv devolvió %d", r);
  //if(r == -1) perror("recv");
	if(r > 0){
    return cod_op;
  } else {
		close(socket_cliente);
		return -1;
	};
};

int crear_conexion(char *ip, char* puerto, int header_cliente){
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

  //log_debug(logger, "Esperando servidor...");
  while (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1) {
		sleep(1);
  };

	freeaddrinfo(server_info);
	return socket_cliente;
};

int iniciar_conexion(void* arg){
  char* puerto = (char*)arg;
  int socket_servidor = iniciar_servidor(puerto);
  log_info(logger, "Listo para recibir al cliente");
  int socket_cliente = esperar_cliente(socket_servidor);
  return socket_cliente;
};

//// FINALIZAR
void terminar_programa(int conexion, t_log* logger, t_config* config){
	close(conexion);
	log_destroy(logger);
	config_destroy(config);
};