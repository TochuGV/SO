#ifndef CICLO_INSTRUCCION_H_
#define CICLO_INSTRUCCION_H_

#include "./utils/utils.h"
#include "estructuras.h"
#include "ciclo_traduccion.h"

//Ciclo completo de instrucción
void* ciclo_de_instruccion(t_cpu*,t_pcb*, int, int,int);

//Manejo de PCB
t_pcb* recibir_pcb(int);
t_pcb* deserializar_pcb(void*);

//Manejo de instrucción
t_list* recibir_instruccion(t_pcb*, int);
t_estado_ejecucion trabajar_instruccion (t_cpu*,t_instruccion, t_pcb*);
void actualizar_kernel(t_instruccion,t_estado_ejecucion, t_pcb*,int);
bool chequear_interrupcion(int, uint32_t);

//Ejecución de instrucción;
void ejecutar_read(t_cpu*,uint32_t,char*,char*);
void ejecutar_write(t_cpu*,uint32_t,char*,char*); 

//Envío de actualizaciónes a Kernel
void agregar_syscall_a_paquete(t_paquete*, uint32_t, uint32_t, char*, char*, uint32_t);
void llenar_paquete (t_paquete*,t_pcb*);

#endif /* CICLO_INSTRUCCION_H_ */
