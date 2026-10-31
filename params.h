#ifndef PARAMS_H
#define PARAMS_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>


// Тип уравнения 
enum {
    EQ_TRANSPORT = 0,   // уравнение переноса      
    EQ_BURGERS   = 1    // уравнение Бюргерса      
};

// Тип граничных условий 
enum {
    BC_NONREFLECTING = 0,   // d/dx = 0        
    BC_PERIODIC      = 1    // периодическое   
};

// Тип начального условия 
enum {
    IC_STEP     = 0,   // ступенька               
    IC_TRIANGLE = 1,   // треугольник             
    IC_GAUSS    = 2,   // гауссова волна / синус  
    IC_LINEAR   = 3    // y = kx + b              
};

typedef struct {
    int    equation_type;   // 0 — перенос, 1 — Бюргерс              
    double a;               // скорость переноса (для переноса)      
    double x_min, x_max;    // границы расчётной области             
    double t_max;           // время окончания расчёта              
    double t_exp;           // период вывода                        
    int    boundary_type;   // 0 — d/dx=0, 1 — периодическое        
    int    u0_type;         // 0..3 — см. выше                      
    double u0_par1;         // центр / матожидание / k              
    double u0_par2;         // высота / дисперсия / b               
    double u0_par3;         // ширина                                
    int    N;               // число отрезков разбиения              
    double Co;              // число Куранта, 0 < Co <= 1            
} Params;

// Обрезает пробелы в начале и в конце строки. Возвращает указатель на начало. 
char *trim(char *s);

// Заполнить структуру значениями по умолчанию.
void params_set_default(Params *p);

// Прочитать параметры из файла не учитывая комментарии. Возвращает 0 при успехе, -1 при ошибке.
int params_read_file(const char *filename, Params *p);

// Проверить корректность параметров. Возвращает 1, если всё хорошо. 
int params_validate(const Params *p);

// Напечатать параметры (для проверки).
void params_print(const Params *p);

double u0(double x, const Params *p);

double exact_solution(double x, double t, const Params *p);

void write_state(FILE *f, const double *u, const Params *p, double t);

#endif // PARAMS_H 