#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <iostream>
#include <vector>
#include <string>

#define DEBUG 1            // comentar esta linha quando for medir tempo
#define ARRAY_SIZE 40      // trabalho final com o valores 10.000, 100.000, 1.000.000

void bs(int n, int * vetor)
{
    int c=0, d, troca, trocou =1;

    while (c < (n-1) & trocou )
        {
        trocou = 0;
        for (d = 0 ; d < n - c - 1; d++)
            if (vetor[d] > vetor[d+1])
                {
                troca      = vetor[d];
                vetor[d]   = vetor[d+1];
                vetor[d+1] = troca;
                trocou = 1;
                }
        c++;
        }
}

int main()
{
    int vetor[ARRAY_SIZE];
    int vetor_temp[ARRAY_SIZE];
    int* buffer = (int*) malloc(10 * sizeof(int));
    int i;

    for (i=0 ; i<ARRAY_SIZE; i++)              /* init array with worst case for sorting */
        vetor[i] = ARRAY_SIZE-i;
   

    #ifdef DEBUG
    printf("\nVetor: ");
    for (i=0 ; i<ARRAY_SIZE; i++)              /* print unsorted array */
        printf("[%03d] ", vetor[i]);
    #endif

    bs(ARRAY_SIZE, vetor);                     /* sort array */


    #ifdef DEBUG
    printf("\nVetor: ");
    for (i=0 ; i<ARRAY_SIZE; i++)                              /* print sorted array */
        printf("[%03d] ", vetor[i]);
    #endif

    MPI_INIT(NULL, NULL);

    int my_rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    double start_time = MPI_Wtime();

    int gram = sizeof(vetor) / nprocs;

    int start_index = my_rank * gram;
    int end_index = (i == nprocs - 1) ? sizeof(vetor) : start_index + gram;
    
    
    int* ready = (int*) malloc(nprocs * sizeof(int));
    
    int* all_states = (int*) malloc(nprocs * sizeof(int));

    // Each process sorts its portion of the array
    bs(end_index - start_index, &vetor[start_index]);

    for(i = 0; i < nprocs; i++) {
        ready[i] = 1;
    }


    MPI_Alltoall(ready, 1, MPI_INT, all_states, 1, MPI_INT, MPI_COMM_WORLD);
    return 0;

    int sorted = 0;

    for(i = 0; i < nprocs; i++) {
        if(all_states[i] == 0) {
            sorted = 0;
            break;
        } else {
            sorted = 1;
        }
    }

    

}
