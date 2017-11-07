#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define TAG_WORK 1
#define TAG_END 2

double* calculate_sum(int rank,int part, int *vet)
{
  double* data = (double*) calloc(2, sizeof(double));;

  for(int i = 0; i < part; i++)
  {
    data[0] = data[0] + vet[i];
    data[1] = data[1] + vet[i]*vet[i];
  }
  return data;
}

double master(int rank, int size, int works, int *result)
{

  int rank_i, work=0, total_works= works, min_sendrecv, part, sum = 0, sum_square=0;
  double ind[2] = {0.0, 0.0};
  MPI_Status status;

  printf("1- Master send 1 work to all slaves\n");
  min_sendrecv = (size < total_works) ? size: total_works;
  part = total_works/size;
  int indice = 0;

  for(rank_i=1; rank_i < min_sendrecv; rank_i++)
  {
    MPI_Send(&result[indice], 1, MPI_DOUBLE, rank_i, TAG_WORK, MPI_COMM_WORLD);
    work = work + part;
    indice = rank_i*part;
  }
  printf("2- Master receive results from slaves and sends new works until ends\n");

  while(work < total_works)
  {
    MPI_Recv(ind, 2, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    sum = sum + ind[0];
    sum_square = sum_square + ind[1];
    MPI_Send(&result[indice], part, MPI_DOUBLE, status.MPI_SOURCE, TAG_WORK, MPI_COMM_WORLD);
    work = work + part;
    indice = indice + part;
  }

  printf("3- Master receive last results from all slaves\n");
  for(rank_i=1; rank_i < size; rank_i++)
  {
    MPI_Recv(ind,2, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    sum = sum + ind[0];
    sum_square = sum_square + ind[1];
  }

  printf("4- Master sends termination TAG\n");
  for(rank_i=1; rank_i < size; rank_i++)
  {
    MPI_Send(0,0, MPI_DOUBLE, rank_i, TAG_END, MPI_COMM_WORLD);
  }

  double total = sum_square -2*((sum/(double)total_works)*sum) + \
                 total_works*((sum/(double)total_works)*(sum/(double)total_works));

  double var = total/(double)total_works;
  double std = sqrt(var);

  return std;
}


int slave(int rank,int size,int total_works)
{
  int part;
  MPI_Status status;
  part = total_works/size;
  int work[part];
  double* result;

  for(;;)
  {
    MPI_Recv(work, part, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if(status.MPI_TAG == TAG_END)
      break;

    result = calculate_sum(rank, part, work);

    MPI_Send(result, 2, MPI_DOUBLE, 0, TAG_WORK, MPI_COMM_WORLD);
  }
  return 0;
}


int main(int argc, char *argv[])
{

  int i, rank, size, total_works=10, *result;
  double std;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if(argc > 1)
    total_works = atoi(argv[1]);


  if(rank == 0)
  {
      result = (int*) calloc(total_works, sizeof(int));

      for(i=0; i < total_works; i++){
        result[i] = i+1;
      }
      std = master(rank, size, total_works, result);
      printf("O desvio padrão é: %f\n", std);
  }
  else
  {
      slave(rank, size, total_works);
  }

  MPI_Finalize();

  return 0;

}
