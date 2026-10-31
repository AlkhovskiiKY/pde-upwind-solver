#include "params.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


char *trim(char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;

    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) *end-- = '\0';//сначала положим '0', а потом сдвинем
    return s;
}


void params_set_default(Params *p) 
{
    p->equation_type = EQ_TRANSPORT;
    p->a             = 1.0;
    p->x_min         = 0.0;
    p->x_max         = 1.0;
    p->t_max         = 1.0;
    p->t_exp         = 0.1;
    p->boundary_type = BC_NONREFLECTING;
    p->u0_type       = IC_STEP;
    p->u0_par1       = 0.5;    // центр ступеньки
    p->u0_par2       = 1.0;    // высота ступеньки
    p->u0_par3       = 0.1;    // ширина ступеньки
    p->N             = 100;
    p->Co            = 0.5;
}

int params_read_file(const char *filename, Params *p) 
{
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "params_read_file: can't open '%s'\n", filename);
        return -1;
    }

    params_set_default(p);

    char line[256];
    int  lineno    = 0;
    int  n_errors  = 0;

    while (fgets(line, sizeof(line), f)) {
        lineno++;

        // отрезаем комментарий после '#' 
        char *hash = strchr(line, '#');
        if (hash) *hash = '\0';

        char *s = trim(line);
        if (*s == '\0') continue;

        // разделитель — '=' или первый пробел/табуляция 
        char *sep = strchr(s, '=');
        if (!sep) sep = strpbrk(s, " \t");
        if (!sep) {
            fprintf(stderr, "params: line %d: without breakpoint: '%s'\n", lineno, s);
            n_errors++;
            continue;
        }
        *sep = '\0';
        char *key = trim(s);
        char *val = trim(sep + 1);

        int handled = 1;//проверка правильности ввода параметров
        if      (!strcmp(key, "equation_type")) p->equation_type = atoi(val);//aSKII tO iNT
        else if (!strcmp(key, "a"))             p->a             = atof(val);//aSKII tO fLOAT
        else if (!strcmp(key, "x_min"))         p->x_min         = atof(val);
        else if (!strcmp(key, "x_max"))         p->x_max         = atof(val);
        else if (!strcmp(key, "t_max"))         p->t_max         = atof(val);
        else if (!strcmp(key, "t_exp"))         p->t_exp         = atof(val);
        else if (!strcmp(key, "boundary_type")) p->boundary_type = atoi(val);
        else if (!strcmp(key, "u0_type"))       p->u0_type       = atoi(val);
        else if (!strcmp(key, "u0_par1"))       p->u0_par1       = atof(val);
        else if (!strcmp(key, "u0_par2"))       p->u0_par2       = atof(val);
        else if (!strcmp(key, "u0_par3"))       p->u0_par3       = atof(val);
        else if (!strcmp(key, "N"))             p->N             = atoi(val);
        else if (!strcmp(key, "Co"))            p->Co            = atof(val);
        else handled = 0;

        if (!handled) {
            fprintf(stderr, "params: line %d: uknown key '%s'\n", lineno, key);
            n_errors++;
        }
    }
    fclose(f);

    if (n_errors > 0) return -1;
    return params_validate(p) ? 0 : -1;
}

int params_validate(const Params *p) {
    int ok = 1;

    if (p->equation_type != EQ_TRANSPORT && p->equation_type != EQ_BURGERS) {
        fprintf(stderr, "params: equation_type must be 0 or 1\n");
        ok = 0;
    }
    if (p->x_min >= p->x_max) {
        fprintf(stderr, "params: need x_min < x_max\n");
        ok = 0;
    }
    if (p->t_max <= 0.0) {
        fprintf(stderr, "params: t_max must be > 0\n");
        ok = 0;
    }
    if (p->t_exp <= 0.0) {
        fprintf(stderr, "params: t_exp must be > 0\n");
        ok = 0;
    }
    if (p->t_exp > p->t_max) {
        fprintf(stderr, "params: t_exp less or equal t_max\n");
        ok = 0;
    }
    if (p->boundary_type != BC_NONREFLECTING && p->boundary_type != BC_PERIODIC) {
        fprintf(stderr, "params: boundary_type must be 0 or 1\n");
        ok = 0;
    }
    if (p->u0_type < 0 || p->u0_type > 3) {
        fprintf(stderr, "params: u0_type must be in interval 0..3\n");
        ok = 0;
    }
    if (p->N < 2) {
        fprintf(stderr, "params: N must be >= 2\n");
        ok = 0;
    }
    if (!(p->Co > 0.0 && p->Co <= 1.0)) {
        fprintf(stderr, "params: number Corant Co must be in (0, 1]\n");
        ok = 0;
    }
    return ok;
}

void params_print(const Params *p) {
    printf("Params of problem:\n");
    printf("  equation_type  = %d (%s)\n", p->equation_type,
           p->equation_type == EQ_TRANSPORT ? "transport" : "Burgers");
    if (p->equation_type == EQ_TRANSPORT)
        printf("  a              = %g\n", p->a);
    printf("  x_min, x_max   = %g, %g\n", p->x_min, p->x_max);
    printf("  t_max, t_exp   = %g, %g\n", p->t_max, p->t_exp);
    printf("  boundary_type  = %d (%s)\n", p->boundary_type,
           p->boundary_type == BC_NONREFLECTING ? "d/dx=0" : "periodic");
    printf("  u0_type        = %d\n", p->u0_type);
    printf("  u0_par1        = %g\n", p->u0_par1);
    printf("  u0_par2        = %g\n", p->u0_par2);
    printf("  u0_par3        = %g\n", p->u0_par3);
    printf("  N              = %d\n", p->N);
    printf("  Co             = %g\n", p->Co);
}


/* ------------------------------------------------------------------ */
/* Вывод одного временного слоя в CSV (столбец t, X, U, U_exact)       */
/* ------------------------------------------------------------------ */
void write_state(FILE *f, const double *u, const Params *p, double t)
{
    double dx = (p->x_max - p->x_min) / p->N;

    for (int k = 0; k <= p->N; ++k) {
        double x     = p->x_min + k * dx;
        double exact = exact_solution(x, t, p);
        fprintf(f, "%.8f,%.8f,%.8f,%.8f\n", t, x, u[k], exact);
    }
}