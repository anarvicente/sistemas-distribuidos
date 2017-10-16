#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

#define TAG_WORK 1
#define TAG_END 2



int found_element(int rank, int limits[2], int element, int *result)
{
  int i = limits[0], found=0;

  for(; i < limits[1]; i++){
    if(result[i] == element)
      found = i;

  }
  return found;
}

int master(int rank, int size, int works, int *result)
{
  int ind=0, rank_i, work=0, total_works= works, min_sendrecv;
  MPI_Status status;
  int vet_pos[2], pos = -1;

  printf("1- Master send 1 work to all slaves\n");
  min_sendrecv = (size < total_works) ? size: total_works;
  int part = total_works/size;
  vet_pos[0] = work;

  for(rank_i=1; rank_i < min_sendrecv; rank_i++)
  {

    vet_pos[1] = vet_pos[0] + part;  /* Limite superior */

    MPI_Send(&vet_pos, 2, MPI_INT, rank_i, TAG_WORK, MPI_COMM_WORLD);
    vet_pos[0] = vet_pos[1];        /* Limite inferior */
    work = work + part;
  }
  printf("2- Master receive results from slaves and sends new works until ends\n");

  while((work < total_works) && !ind)
  {
    MPI_Recv(&ind, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if(ind)
      pos = ind;

    vet_pos[1] = vet_pos[0] + part;
    MPI_Send(&vet_pos, 2, MPI_INT, status.MPI_SOURCE, TAG_WORK, MPI_COMM_WORLD);

    vet_pos[0] = vet_pos[1];
    work = work + part;
  }

  printf("3- Master receive last results from all slaves\n");
  for(rank_i=1; rank_i < size; rank_i++)
  {
    MPI_Recv(&ind,1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if(ind)
      pos = ind;

  }

  printf("4- Master sends termination TAG\n");
  for(rank_i=1; rank_i < size; rank_i++)
  {
    MPI_Send(0,0, MPI_INT, rank_i, TAG_END, MPI_COMM_WORLD);
  }

  return pos;
}


int slave(int rank, int *result)
{
  int work[2], found = 0;
  MPI_Status status;

  for(;;)
  {
    MPI_Recv(&work, 2, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if(status.MPI_TAG == TAG_END)
      break;

    /* Faz um processamento */
    found = found_element(rank, work, 10, result);
    /* Faz um processamento */

    MPI_Send(&found, 1, MPI_INT, 0, TAG_WORK, MPI_COMM_WORLD);
  }
  return 0;
}


int main(int argc, char *argv[])
{

  int  rank, size, total_works=10, *result;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if(argc > 1)
    total_works = atoi(argv[1]);

  result = (int*) calloc(total_works, sizeof(int));    /* vetor global */
  for(int i=0; i < total_works; i++){                  /* Encher o vetor */
    result[i] = i+1;
  }

  if(rank == 0)
  {

      int pos = master(rank, size, total_works, result);
      printf("POS DO ELEMENTO:%d\n", pos);

  }
  else
  {
      slave(rank, result);
  }

  MPI_Finalize();
  return 0;

}
