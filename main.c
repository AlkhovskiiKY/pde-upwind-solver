#include "params.h"

int main(int argc, char **argv)
{
    Params p;

    const char *fname = (argc > 1) ? argv[1] : "params.txt";

    if (params_read_file(fname, &p) != 0) {
        fprintf(stderr, "Can't read parameters from '%s'\n", fname);
        return 1;
    }

    params_print(&p);

    // Шаг по пространству 
    double dx = (p.x_max - p.x_min) / p.N;

    // Выделение памяти 
    double *u  = (double *)malloc((p.N + 1) * sizeof(double));
    double *u1 = (double *)malloc((p.N + 1) * sizeof(double));
    if (!u || !u1) {
        fprintf(stderr, "Memory allocation error\n");
        return 1;
    }

    // Заполнение начального условия 
    for (int k = 0; k <= p.N; ++k) {
        double x = p.x_min + k * dx;
        u[k] = u0(x, &p);
    }

    FILE *fout = fopen("result.csv", "w");
    if (!fout) {
        fprintf(stderr, "Can't open result.csv for writing\n");
        free(u); free(u1);
        return 1;
    }

    // Шапка таблицы 
    fprintf(fout, "t,X,U,U_exact\n");

    double current_time = 0.0;
    double next_output  = p.t_exp;

    // ---- Начальный вывод ----
    write_state(fout, u, &p, current_time);

    clock_t start_cpu = clock();

    // ---- Основной цикл по времени ---- 
    while (current_time < p.t_max - 1e-12) {

        // Максимальная скорость распространения волн 
        double s_max = 0.0;
        if (p.equation_type == EQ_TRANSPORT) {
            s_max = fabs(p.a);
        } else {
            for (int k = 0; k <= p.N; ++k) {
                double abs_u = fabs(u[k]);
                if (abs_u > s_max) s_max = abs_u;
            }
        }
        if (s_max < 1e-12) break;

        // Шаг по времени из условия CFL 
        double dt = p.Co * dx / s_max;
        if (current_time + dt > p.t_max)
            dt = p.t_max - current_time;
        if (dt <= 0.0) break;

        // Новый временной слой 
        for (int k = 1; k < p.N; ++k) {
            if (p.equation_type == EQ_TRANSPORT) {
                if (p.a >= 0.0) {
                    double f_k   = p.a * u[k];
                    double f_km1 = p.a * u[k-1];
                    u1[k] = u[k] - dt / dx * (f_k - f_km1);
                } else {
                    double f_kp1 = p.a * u[k+1];
                    double f_k   = p.a * u[k];
                    u1[k] = u[k] - dt / dx * (f_kp1 - f_k);
                }
            } else {
                if (u[k] >= 0.0) {
                    double f_k   = 0.5 * u[k] * u[k];
                    double f_km1 = 0.5 * u[k-1] * u[k-1];
                    u1[k] = u[k] - dt / dx * (f_k - f_km1);
                } else {
                    double f_kp1 = 0.5 * u[k+1] * u[k+1];
                    double f_k   = 0.5 * u[k] * u[k];
                    u1[k] = u[k] - dt / dx * (f_kp1 - f_k);
                }
            }
        }

        u1[0]   = u[0];
        u1[p.N] = u[p.N];

        for (int k = 0; k <= p.N; ++k)
            u[k] = u1[k];

        // Граничные условия 
        if (p.boundary_type == BC_NONREFLECTING) {
            u[0]   = u[1];
            u[p.N] = u[p.N - 1];
        } else {
            u[0]   = u[p.N - 1];
            u[p.N] = u[1];
        }

        current_time += dt;

        // Периодический вывод в тот же файл s
        if (current_time >= next_output - 1e-12 ||
            current_time >= p.t_max     - 1e-12) {

            write_state(fout, u, &p, current_time);

            next_output += p.t_exp;
            if (next_output < current_time)
                next_output = current_time + p.t_exp;
        }
    }

    clock_t end_cpu = clock();
    double cpu_time = (double)(end_cpu - start_cpu) / CLOCKS_PER_SEC;

    fclose(fout);
    free(u);
    free(u1);

    printf("Calculation finished. CPU time = %.3f sec.\n", cpu_time);
    printf("Results written to result.csv\n");

    return 0;
}