#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

#define TAG_WORK 1
#define TAG_END 2
#define ELEMENT 5


int found_element(int rank,int part, int *vet, int element)
{
  int found = 0;
  for(int i = 0; i < part; i++)
  {
    if(vet[i] == element)
    {
      found = 1;
      break;
    }
  }
  return found;
}


int master(int rank, int size, int works, int *result)
{
  int ind=0, rank_i, work=0, total_works= works, min_sendrecv, part;
  MPI_Status status;

  printf("1- Master send 1 work to all slaves\n");
  min_sendrecv = (size < total_works) ? size: total_works;
  part = total_works/size;
  int indice = 0;

  for(rank_i=1; rank_i < min_sendrecv; rank_i++)
  {
    MPI_Send(&result[indice], part, MPI_INT, rank_i, TAG_WORK, MPI_COMM_WORLD);
    work = work + part;
    indice = rank_i*part;
  }
  printf("2- Master receive results from slaves and sends new works until ends\n");

  while((work < total_works) && (!ind))
  {
    MPI_Recv(&ind, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    MPI_Send(&result[indice], part, MPI_INT, status.MPI_SOURCE, TAG_WORK, MPI_COMM_WORLD);
    work = work + part;
    indice = indice + part;
  }

  printf("3- Master receive last results from all slaves\n");
  for(rank_i=1; rank_i < size; rank_i++)
  {
    MPI_Recv(&ind,1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

  }

  printf("4- Master sends termination TAG\n");
  for(rank_i=1; rank_i < size; rank_i++)
  {
    MPI_Send(0,0, MPI_INT, rank_i, TAG_END, MPI_COMM_WORLD);
  }

  return 0;
}


int slave(int rank,int size,int total_works,int element)
{
  int result, part;
  MPI_Status status;
  part = total_works/size;
  int work[part];

  for(;;)
  {
    MPI_Recv(work, part, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if(status.MPI_TAG == TAG_END)
      break;

    /* Faz um processamento */
    result = found_element(rank,part, work, element);
    /* Faz um processamento */

    MPI_Send(&result, 1, MPI_INT, 0, TAG_WORK, MPI_COMM_WORLD);
  }
  return 0;
}


int main(int argc, char *argv[])
{

  int i, rank, size, total_works=10, *result;

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

      /* TODO: tentar calcular a posicao do elemento no vetor global
         Calcular o desvio padrao
      */
      master(rank, size, total_works, result);
  }
  else
  {
      slave(rank, size, total_works, ELEMENT);
  }

  MPI_Finalize();
  //free(result);
  return 0;

}
