#ifndef SYSCALLS_H_
#define SYSCALLS_H_

#include "utils/utils.h"
#include "planificacion/planificacion.h"

typedef struct {
  uint32_t pid;
  tipo_syscall tipo;
  char* arg1;
  char* arg2;
  uint32_t pc;
} t_syscall;

t_syscall* recibir_syscall(int);
void syscall_init_proc(t_syscall*);
void syscall_exit(t_syscall*);
void syscall_io(t_syscall*);
void syscall_dump_memory(t_syscall*);
void manejar_syscall(t_syscall*, int);
void destruir_syscall(t_syscall*);

#endif /* SYSCALLS_H_ */