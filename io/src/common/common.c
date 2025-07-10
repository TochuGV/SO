#include "common.h"
#include <ctype.h>

int32_t handshake_io(char* nombre, int conexion_kernel){
  int32_t tipo = MODULO_IO;
  int32_t token_io;
  int32_t resultado;

  if (strcasecmp(nombre, "impresora") == 0)
    token_io = IMPRESORA;
  else if (strcasecmp(nombre, "teclado") == 0)
    token_io = TECLADO;
  else if (strcasecmp(nombre, "mouse") == 0)
    token_io = MOUSE;
  else if (strcasecmp(nombre, "auriculares") == 0)
    token_io = AURICULARES;
  else if (strcasecmp(nombre, "parlante") == 0)
    token_io = PARLANTE;
  else if (strcasecmp(nombre, "disco") == 0)
    token_io = DISCO;
  else {
    log_error(logger, "Nombre de dispositivo IO desconocido: %s", nombre);
    return -1;
  };

  send(conexion_kernel, &tipo, sizeof(int32_t), 0);
  send(conexion_kernel, &token_io, sizeof(int32_t), 0);
  if (recv(conexion_kernel, &resultado, sizeof(int32_t), MSG_WAITALL) <= 0){
    log_error(logger, "No se pudo recibir respuesta del Kernel");
    return -1;
  };

  if (resultado == -1) {
    log_error(logger, "La conexión con Kernel falló. Finalizando conexión...");
    return -1;
  } else {
    convertir_primera_letra_en_mayuscula(nombre);
    log_debug(logger, "%s se ha conectado a Kernel exitosamente!", nombre);
    return 0;
  };
};

nombre_dispositivo_io obtener_dispositivo_io(char* nombre){
  if (strcasecmp(nombre, "impresora") == 0) return IMPRESORA;
  else if (strcasecmp(nombre, "teclado") == 0) return TECLADO;
  else if (strcasecmp(nombre, "mouse") == 0) return MOUSE;
  else if (strcasecmp(nombre, "auriculares") == 0) return AURICULARES;
  else if (strcasecmp(nombre, "parlante") == 0) return PARLANTE;
  else if (strcasecmp(nombre, "disco") == 0) return DISCO;
  else {
    log_error(logger, "Nombre de dispositivo IO desconocido: %s", nombre);
    return -1;
  };
};

void atender_interrupcion(int conexion_kernel) {

  t_list* lista = recibir_paquete(conexion_kernel);
  uint32_t pid = *(uint32_t*)list_get(lista, 0);
  uint32_t tiempo_io = *(uint32_t*)list_get(lista, 1);
  
  log_inicio_io(pid, tiempo_io);
  usleep(tiempo_io * 1000);
  log_finalizacion_io(pid);
  
  t_paquete* paquete = crear_paquete(FINALIZACION_IO);
  agregar_a_paquete(paquete, &pid, sizeof(uint32_t));
  enviar_paquete(paquete, conexion_kernel);
};

void convertir_primera_letra_en_mayuscula(char* cadena){
  if(cadena == NULL || cadena[0] == '\0') return;
  cadena[0] = toupper(cadena[0]);
};