#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


#define TAG_WORK 1
#define TAG_END 2
#define TAG_STD 3

double calculate_sum(int rank,int part, int *vet)
{
  double sum = 0;
  for(int i = 0; i < part; i++)
  {
    sum = sum + vet[i];
  }
  return sum;
}

double square_deviation(int part, int *vet, double media_geral)
{
  double sub = 0.0;
  double square = 0.0;
  for(int i=0; i < part; i++)
  {
    sub = vet[i]-media_geral;
    square = square + sub*sub;
  }
  return square;
}


double master(int rank, int size, int works, int *result)
{
  int rank_i, work=0, total_works= works, min_sendrecv, part;
  double media_geral = 0.0, ind = 0.0, sum = 0.0;
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
    MPI_Recv(&ind, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    sum = sum + ind;
    MPI_Send(&result[indice], part, MPI_DOUBLE, status.MPI_SOURCE, TAG_WORK, MPI_COMM_WORLD);
    work = work + part;
    indice = indice + part;
  }

  printf("3- Master receive last results from all slaves\n");
  for(rank_i=1; rank_i < size; rank_i++)
  {
    MPI_Recv(&ind,1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    sum = sum + ind;
  }

  media_geral = sum/(double)total_works;
  printf("MEDIA GERAL: %f\n", media_geral);


  /* DESVIO PADRAO */

  printf("4- Master send 1 work to all slaves\n");
  min_sendrecv = (size < total_works) ? size: total_works;
  part = total_works/size;
  indice = 0; work = 0; sum=0.0; ind=0.0;

  for(rank_i=1; rank_i < min_sendrecv; rank_i++)
  {
    MPI_Send(&result[indice], 1, MPI_DOUBLE, rank_i, TAG_STD, MPI_COMM_WORLD);
    work = work + part;
    indice = rank_i*part;
  }

  printf("5- Master receive results from slaves and sends new works until ends\n");
  while(work < total_works)
  {
    MPI_Recv(&ind, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    sum = sum + ind;
    MPI_Send(&result[indice], part, MPI_DOUBLE, status.MPI_SOURCE, TAG_STD, MPI_COMM_WORLD);
    work = work + part;
    indice = indice + part;
  }

  printf("6- Master receive last results from all slaves\n");
  for(rank_i=1; rank_i < size; rank_i++)
  {
    MPI_Recv(&ind,1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    sum = sum + ind;
  }

  printf("7- Master sends termination TAG\n");
  for(rank_i=1; rank_i < size; rank_i++)
  {
    MPI_Send(0,0, MPI_DOUBLE, rank_i, TAG_END, MPI_COMM_WORLD);
  }


  double var = sum/(double)total_works;
  double std = sqrt(var);

  return std;

}


int slave(int rank,int size,int total_works)
{
  int part;
  MPI_Status status;
  part = total_works/size;
  int work[part];
  double  result, media_geral=5.5;

  for(;;)
  {
    MPI_Recv(work, part, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if(status.MPI_TAG == TAG_END)
      break;

    if(status.MPI_TAG == TAG_STD){
      result = square_deviation(part, work, media_geral);
    }else{
      result = calculate_sum(rank,part, work);
    }

    MPI_Send(&result, 1, MPI_DOUBLE, 0, TAG_WORK, MPI_COMM_WORLD);
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

      /* Preencher o vetor randomicamente */
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
