/* Exercício MPI em aula
	Lucas Alexandre Soares 	9293265
	Elisa Saltori Trujillo 	8551100
	Eder Rosati Ribeiro		8122585
	Luiz Eduardo Dorici		4165850
	Fulvio Eduardo Ferreira	8921051
	Douglas Seiti Kodama	9277131
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <mpi.h>

#define VEC_SIZE 124567

typedef enum { false, true } bool;

void kill(int mpi_error){
	MPI_Abort(MPI_COMM_WORLD, mpi_error);
	MPI_Finalize();
	exit(1);
}

void printvec(int *v, int n){
	
	int i;
	
	printf("v = {");
	for(i = 0; i < n; i++)
		printf("%d, ", v[i]);
	printf("\b\b}\n");
}

int main(int argc, char *argv[]){

	int nproc, rank;
	int i = 0;
	int number, repetitions = 0;
	int *vector = (int *) malloc(sizeof(int)*VEC_SIZE);
	int *recv = NULL;

	int *sendVec;
	int *displacement;

	// Init
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nproc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int count = VEC_SIZE/nproc;
	int rest = VEC_SIZE%nproc;
	sendVec = (int *) malloc(sizeof(int)*nproc);
	displacement = (int *) malloc(sizeof(int)*nproc);
	
	int sum = 0;
	for(i = 0; i < nproc; i++){
		
		sendVec[i] = count;
		if(rest > 0)
			sendVec[i]++;

		displacement[i] = sum;
		sum += sendVec[i];
	}

	if(rank == 0){
		i = 0;
		
		// Get vector - only root
		srand(time(NULL));
		while(i < VEC_SIZE) vector[i++] = rand()%10;

		// Get search number
		scanf("%d", &number);
		
		printvec(vector, VEC_SIZE);
	}


	// Send search number to all processes
	MPI_Bcast(&number, 1, MPI_INTEGER, 0, MPI_COMM_WORLD);

	recv = (int *) malloc(sizeof(int)*VEC_SIZE/nproc);

	// Send vector - scatterv support non integer workload division
	MPI_Scatterv(vector, sendVec, displacement, MPI_INTEGER, 
				 recv, sendVec[rank], MPI_INTEGER, 0, MPI_COMM_WORLD);
	// Send vector - this scatter must have a divisible vector
	/*MPI_Scatter(vector, count, MPI_INTEGER, recv, count, 
				MPI_INTEGER, 0, MPI_COMM_WORLD);*/
	
	// No need for barrier

	// Start searching
	#pragma omp parallel for reduction(+:repetitions)
	for(i = 0; i < sendVec[rank]; i++){
		if(recv[i] == number){ // Found number, increment repetition counter
			repetitions++;
#ifdef debug
			fprintf(stderr, "[debug] Found number %d %d times on process %d\n", 
					number, repetitions, rank);
#endif
		}
	}

	// Reduce repetitions
	int result;
	MPI_Reduce(&repetitions, &result, 1, MPI_INTEGER, MPI_SUM, 0, MPI_COMM_WORLD);

	if(rank == 0){
		printf("Found number %d %d times\n", number, result);
		free(vector);
	}

	MPI_Finalize();
	free(recv);
	free(sendVec);
	free(displacement);

	return 0;
}