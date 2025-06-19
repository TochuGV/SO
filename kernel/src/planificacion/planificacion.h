#ifndef PLANIFICACION_H_
#define PLANIFICACION_H_

#include "planificadores/largo_plazo/largo_plazo.h"
#include "planificadores/corto_plazo/corto_plazo.h"
#include "estados/estados.h"

void* planificador_largo_plazo(void*);
void* planificador_corto_plazo(void*);

#endif