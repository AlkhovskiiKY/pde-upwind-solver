#include "params.h"

/* ------------------------------------------------------------------ */
/* Начальное условие                                                   */
/* ------------------------------------------------------------------ */
double u0(double x, const Params *p)
{
    switch (p->u0_type) {
        case IC_STEP:
            // ступенька: центр par1, высота par2, ширина par3 
            if (fabs(x - p->u0_par1) <= p->u0_par3 / 2.0)
                return p->u0_par2;
            else
                return 0.0;

        case IC_TRIANGLE:
            // равнобедренный треугольник: центр par1, высота par2, ширина par3 
            if (fabs(x - p->u0_par1) <= p->u0_par3 / 2.0)
                return p->u0_par2 * (1.0 - 2.0 * fabs(x - p->u0_par1) / p->u0_par3);
            else
                return 0.0;

        case IC_GAUSS:
            // гауссова волна: матожидание par1, дисперсия par2 
            return exp(-((x - p->u0_par1) * (x - p->u0_par1)) / (2.0 * p->u0_par2));

        case IC_LINEAR:
            // линейная функция y = kx + b, где k = par1, b = par2 
            return p->u0_par1 * x + p->u0_par2;

        default:
            return 0.0;
    }
}

/* ------------------------------------------------------------------ */
/* Точное решение (там, где известно)                                  */
/* ------------------------------------------------------------------ */
double exact_solution(double x, double t, const Params *p)
{
    if (p->equation_type == EQ_TRANSPORT) {
        // u(x,t) = u0(x - a*t) 
        double x0 = x - p->a * t;
        return u0(x0, p);
    } else if (p->equation_type == EQ_BURGERS) {
        // точное решение только для линейного начального условия 
        if (p->u0_type == IC_LINEAR) {
            double k = p->u0_par1;   // коэффициент при x
            double b = p->u0_par2;   // свободный член
            return (k * x + b) / (1.0 + k * t);
        }
    }
    // для остальных случаев точное решение не задано
    return 0.0;
}