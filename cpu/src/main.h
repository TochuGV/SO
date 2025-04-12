#ifndef MAIN_H_
#define MAIN_H_

#include "./utils/utils.h"

t_log* iniciar_logger(char*, char*, t_log_level);
t_config* iniciar_config(char*);


void leer_consola(t_log*);
void paquete(int);
void terminar_programa(int, t_log*, t_config*);
void iterator(char* value);

#endif /* MAIN_H_ */