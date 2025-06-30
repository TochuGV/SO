#ifndef SRT_H_
#define SRT_H_

#include <stdint.h>
#include <stdbool.h>
#include "utils/utils.h"
#include <commons/collections/dictionary.h>
#include "init/init.h"
#include "pcb/pcb.h"
#include "cpu/cpu.h"

bool es_SRT(void);

double tiempo_restante_exec(t_pcb* pcb);
t_pcb* obtener_proceso_en_exec_con_mayor_estimacion(int);
void desalojar_cpu(void);

#endif /* SRT_H_ */
