#include "common_io.h"

int32_t handshake_io(char* nombre, int conexion_kernel)
{
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

  send(conexion_kernel, &token_io, sizeof(int32_t), 0);
  recv(conexion_kernel, &resultado, sizeof(int32_t), MSG_WAITALL);

  if(resultado == -1) {
    log_error(logger, "Error: La conexión con Kernel falló. Finalizando conexión...");
    return -1;
  }
  else {
    log_info(logger," %s se ha conectado a Kernel exitosamente!",nombre);
    return 0;
  }
}


/*atender_interrupcion() recibe solicitudes de IO del Kernel, ejecuta un usleep() para simular la operación y luego notifica al Kernel cuando finaliza. 
También registra logs para monitorear el inicio y fin de cada IO.*/

void atender_interrupcion(int conexion_kernel) {
    int32_t pid;
    int32_t tiempo_io;

    while (1) { // Mantener el módulo IO activo para escuchar solicitudes
        if (recv(conexion_kernel, &pid, sizeof(int32_t), MSG_WAITALL) <= 0) {
            log_error(logger, "Error al recibir PID, desconectando...");
            break;
        }
        if (recv(conexion_kernel, &tiempo_io, sizeof(int32_t), MSG_WAITALL) <= 0) {
            log_error(logger, "Error al recibir tiempo de IO, desconectando...");
            break;
        }

        log_info(logger, "## PID: %d - Inicio de IO - Tiempo: %d ms", pid, tiempo_io);

        usleep(tiempo_io * 1000); // Simular la ejecución de la IO

        log_info(logger, "## PID: %d - Fin de IO", pid);

        send(conexion_kernel, &pid, sizeof(int32_t), 0); // Notificar al Kernel que terminó la IO
    }
}
