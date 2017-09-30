#include <mpi.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv){
	int size, rank;
	char vetor[100];
	
	MPI_Init(&argc, &argv);
	
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	printf("Rank %d de um total de %d\n", rank, size);
	
	if(rank==0){
		strcpy(vetor,"Ola, eu sou o processo 0\n");
	}
	
	MPI_Bcast(&vetor, 100, MPI_CHAR,0, MPI_COMM_WORLD);
	printf("Rank %d de um total de %d\nMensagem: %s\n", rank, size,vetor);

	MPI_Finalize();
	return(0);
}
