#ifndef CONEXION_H_
#define CONEXION_H_

#include "utils/utils.h"
#include "./conexion/handshake/entrante/entrante.h"

void aceptar_conexiones(char* puerto, void* (*atender_cliente)(void*), char* nombre_modulo);
bool validar_handshake_cliente(int socket, int modulo_esperado, char* nombre_modulo);

//Revisar si puede ir 'const char*' en ambos

#endif /* CONEXION_H_ */