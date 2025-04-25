#ifndef INSTRUCCION_H_
#define INSTRUCCION_H_

typedef enum {
	NOOP,
	WRITE,
	READ,
	GOTO,
	IO,
	INIT_PROC,
	DUMP_MEMORY,
	EXIT
} t_tipo_instruccion;

typedef struct {
    t_tipo_instruccion tipo;
    uint32_t parametro1;
    uint32_t parametro2;
} t_instruccion;

#endif /* INSTRUCCION_H_ */