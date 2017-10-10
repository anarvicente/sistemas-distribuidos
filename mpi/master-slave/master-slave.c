#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

#define TAG_WORK 1
#define TAG_END 2


int master(int rank, int size, int works, int *result)
{
  int ind, rank_i, work=0, total_works= works, min_sendrecv;
  MPI_Status status;

  printf("1- Master send 1 work to all slaves\n");
  min_sendrecv = (size < total_works) ? size: total_works;

  for(rank_i=1; rank_i < min_sendrecv; rank_i++)
  {
    MPI_Send(&work, 1, MPI_INT, rank_i, TAG_WORK, MPI_COMM_WORLD);
    work++;
  }
  printf("2- Master receive results from slaves and sends new works until ends\n");

  while(work < total_works)
  {
    MPI_Recv(&ind, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    result[ind] = ind;
    MPI_Send(&work, 1, MPI_INT, status.MPI_SOURCE, TAG_WORK, MPI_COMM_WORLD);
    work++;
  }

  printf("3- Master receive last results from all slaves\n");
  for(rank_i=1; rank_i < size; rank_i++)
  {
    MPI_Recv(&ind,1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    result[ind] = ind;
  }

  printf("4- Master sends termination TAG\n");
  for(rank_i=1; rank_i < size; rank_i++)
  {
    MPI_Send(0,0, MPI_INT, rank_i, TAG_END, MPI_COMM_WORLD);
  }

  return 0;
}


int slave(int rank)
{
  int work, result;
  MPI_Status status;

  for(;;)
  {
    MPI_Recv(&work, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if(status.MPI_TAG == TAG_END)
      break;

    /* Faz um processamento */
    result = work;
    /* Faz um processamento */

    MPI_Send(&result, 1, MPI_INT, 0, TAG_WORK, MPI_COMM_WORLD);
  }
  return 0;
}


int main(int argc, char *argv[])
{

  int i, rank, size, total_works=100, *result;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if(argc > 1)
    total_works = atoi(argv[1]);

  if(rank == 0)
  {
      result = (int*) calloc(total_works, sizeof(int));
      master(rank, size, total_works, result);

      for(i=0; i < total_works; i++)
        printf("res[%d] = %d\t",i, result[i]);
      printf("\n");

  }
  else
  {
      slave(rank);
  }

  MPI_Finalize();
  return 0;

}
