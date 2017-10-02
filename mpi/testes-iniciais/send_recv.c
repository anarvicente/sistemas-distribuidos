#include <mpi.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv){
	int size, rank, ranki;
	int vetor[2];
	/* criar um vetor para send e outro para recv */

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	printf("Rank %d de um total de %d\n", rank, size);

	if(rank==0){
		/*strcpy(vetor,"1\n");*/
		vetor[0]= 1;
		/*O send envia n bytes, no caso, 100. O 123 eh o tag (destino)*/
		for(ranki=1; ranki < size; ranki++){
			MPI_Send(&vetor, 1, MPI_INT, ranki, 123, MPI_COMM_WORLD);
		}
	}
	else{
		MPI_Recv(vetor, 1, MPI_INT, 0, 123, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("Rank1: %d\n", vetor[0]);
	}
	MPI_Finalize();
	return(0);
}
