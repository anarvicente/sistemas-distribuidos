#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

/*=====================================================================
Descricao: Este programa calcula a integral atraves de duas threads
Autor: Sergio Nery Simoes

Para compilar:
    mpicc integral_mpic.c -o integral_mpic

Para executar:
    mpirun -np 4 integral_mpi 0.0 1.0 200000000

========================================================================*/
double integral = 0.0;

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

void CalculaArea(double args[3])
{
    int i;
    double area, dx, x1, x2, f1, f2;
    double a=args[0]; double b=args[1]; double N=args[2];
    printf("a=%10.2lf\tb=%10.2lf\tN=%10.2lf\n", a, b, N);
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

    integral += area;
}

int main(int argc, char **argv)
{
    double a, b, dx;
    int N, i, rank, size;

    MPI_Init(&argc, &argv);

  	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  	MPI_Comm_size(MPI_COMM_WORLD, &size);

    printf("Rank %d de um total de %d\n", rank, size);

    if (argc<4)
    {
        printf("Numero de argumentos insuficiente...\n");
        exit(-1);
    }

    if(rank == 0)
    {
      double args[size][3];

      a = (double) atof(argv[1]);
      b = (double) atof(argv[2]);
      N = atoi(argv[3]);

      dx = (a+b)/size;

      args[0][0] = a;  /* a */
      args[0][1] = dx;  /* b */
      args[0][2] = N/size; /* N */

      for(i=1; i < size; i++){
        args[i][0] = args[i-1][1];
        args[i][1] = args[i][0] + dx;
        args[i][2] = N/size;
      }

      printf("Ola, eu sou o processo 0\n");

  		double vet[1];
  		for(int rank_i=1; rank_i <= size-1; rank_i++)
  		{
  				MPI_Send(&args[rank_i], 3, MPI_DOUBLE, rank_i, 123, MPI_COMM_WORLD);
  		}

  		for(int rank_i=1; rank_i < size; rank_i++)
  		{
  			MPI_Recv(vet,1, MPI_DOUBLE, rank_i, 123, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  			integral = integral + vet[0]; /* recebendo dos outros processos */
  		}

      CalculaArea(args[0]);
      printf("Area= %.15lf\n", integral);
    }
    else
    {
      double vet_recv[size/2][3];
  	  double sum[1];
  		MPI_Recv(vet_recv,3, MPI_DOUBLE, 0, 123, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      CalculaArea(vet_recv[0]);

  		sum[0] = integral;

  		MPI_Send(&sum, 1, MPI_DOUBLE, 0, 123, MPI_COMM_WORLD);

    }

    MPI_Finalize();
    return (0);
}
