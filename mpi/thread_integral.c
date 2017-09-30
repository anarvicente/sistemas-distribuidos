#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <sys/sysinfo.h>

/*=====================================================================
Descricao: Este programa calcula a integral atraves de duas threads
Autor: Sergio Nery Simoes

Para compilar:
    gcc thread_integral.c -o thread_integral -lpthread -lm -Wall

Para executar:
    time ./thread_integral 0.0 1.0 200000000

========================================================================*/

double integral = 0.0;
pthread_mutex_t soma;

typedef struct
{
    double a;
    double b;
    int N;
} Targumentos;

double AreaTrapezio(double dx, double h1, double h2)
{
    return (dx*(h1+h2)/2);
}

double f(double x)
{
    return (4*sqrt(1-x*x));
}

void ThreadCalculaArea(Targumentos *argumentos)
{
    int i;
    double area, dx, x1, x2, f1, f2;
    Targumentos arg;
    arg = *argumentos;
    double a=arg.a; double b=arg.b; int N=arg.N;
    printf("a=%10.2lf\tb=%10.2lf\tN=%d\n", a, b, N);
    dx   = (b-a)/N;
    area = 0.0;
    for (i=0; i<N; i++)
    {
        x1 = a + dx * i;
        x2 = a + dx * (i+1);
        f1 = f(x1);
        f2 = f(x2);
        area += AreaTrapezio(dx, f1, f2);
    }

    pthread_mutex_lock(&soma);
    integral += area;
    pthread_mutex_unlock(&soma);

}

int main(int argc, char **argv)
{
    double a, b, dx;
    int N, n_proc = get_nprocs(), i;
    Targumentos args[n_proc];

    if (argc<4)
    {
        printf("Numero de argumentos insuficiente...\n");
        exit(-1);
    }
    a = (double) atof(argv[1]);
    b = (double) atof(argv[2]);
    N = atoi(argv[3]);


    dx = (a+b)/n_proc;

    args[0].a = a;
    args[0].b = dx;
    args[0].N = N/n_proc;

    for(i=1; i < n_proc; i++){
      args[i].a = args[i-1].b;
      args[i].b = args[i-1].b + dx;
      args[i].N = N/n_proc;
    }

    /*
    arg1.a = a,
    arg1.b = (a+b)/4,
    arg1.N = N/4;

    arg2.a = (a+b)/4,
    arg2.b = (a+b)/4 + (a+b)/4,
    arg2.N = N/4;

    arg3.a = (a+b)/4 + (a+b)/4,
    arg3.b = (a+b)/4 + (a+b)/4 + (a+b)/4,
    arg3.N = N/4;

    arg4.a = (a+b)/4 + (a+b)/4 + (a+b)/4,
    arg4.b = b,
    arg4.N = N/4;
    */

    for(i=0; i < n_proc; i++){
        pthread_create(&threads[i], NULL, (void*)ThreadCalculaArea, &args[i]);
    }

    for(i=0; i<n_proc; i++){
        pthread_join(threads[i], NULL);
    }

    printf("Area= %.15lf\n", integral);
    return 0;
}
