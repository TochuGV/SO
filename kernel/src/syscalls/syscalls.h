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

#endif