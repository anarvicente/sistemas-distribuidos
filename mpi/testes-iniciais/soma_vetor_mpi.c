#include <mpi.h>
#include <stdio.h>
#include <string.h>

/* Para compilar:
	mpicc soma_vetor_mpic.c -o soma_vetor_mpic
	Rodar:
	mpirun -np 4 soma_vetor_mpic
*/

#define TAM 12

int main(int argc, char **argv){
	int size, rank, soma;
	int vet_send[]={1,2,3,4,5,6,7,8,9,10,11,12};
	int vet_recv[TAM/2];

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	printf("Rank %d de um total de %d\n", rank, size);

	if(rank==0)
	{
    printf("Ola, eu sou o processo 0\n");
		int part = TAM/size; /*saber o tamanho dos subvetores */
		int pos_vetor;
		int soma, vet[1];
		for(int rank_i=1; rank_i < size; rank_i++)
		{
				pos_vetor = rank_i*part; /* saber onde comeca cada vetor */
				MPI_Send(&vet_send[pos_vetor], part, MPI_INT, rank_i, 123, MPI_COMM_WORLD);
		}

		for(int i=0; i < part; i++){
			soma = soma + vet_send[i];
		}
		for(int rank_i=1; rank_i < size; rank_i++)
		{
			MPI_Recv(vet,1, MPI_INT, rank_i, 123, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			soma = soma + vet[0];
		}
		printf("TOTAL: %d\n", soma);
	}
	else{
		int part = TAM/size;
		int soma = 0;
		int sum[1];
		MPI_Recv(vet_recv,part, MPI_INT, 0, 123, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		for(int i=0; i<part; i++){
			printf("RECV: %d\n", vet_recv[i]);
			soma = soma + vet_recv[i];
		}
		sum[0] = soma;

		MPI_Send(&sum, 1, MPI_INT, 0, 123, MPI_COMM_WORLD);
	}
	MPI_Finalize();
	return(0);
}
