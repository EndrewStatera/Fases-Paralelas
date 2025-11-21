#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define ARRAY_SIZE 40

void bubble_sort(int n, int *v) {
    int i, j, t;
    for (i = 0; i < n - 1; i++)
        for (j = 0; j < n - i - 1; j++)
            if (v[j] > v[j + 1]) {
                t = v[j];
                v[j] = v[j + 1];
                v[j + 1] = t;
            }
}

// merge two blocks and keep either lower or upper half
void merge_low(int *local, int *recv, int block) {
    int temp[block];
    int i = 0, j = 0, k = 0;
    while (k < block) {
        if (local[i] <= recv[j]) temp[k++] = local[i++];
        else temp[k++] = recv[j++];
    }
    for (i = 0; i < block; i++) local[i] = temp[i];
}

void merge_high(int *local, int *recv, int block) {
    int temp[block];
    int i = block - 1, j = block - 1, k = block - 1;
    while (k >= 0) {
        if (local[i] >= recv[j]) temp[k--] = local[i--];
        else temp[k--] = recv[j--];
    }
    for (i = 0; i < block; i++) local[i] = temp[i];
}

int main(int argc, char *argv[]) {
    int rank, nprocs;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    if (ARRAY_SIZE % nprocs != 0) {
        if (rank == 0) printf("ARRAY_SIZE must be divisible by nprocs!\n");
        MPI_Finalize();
        return 0;
    }

    int block = ARRAY_SIZE / nprocs;
    int *global = NULL;
    int *local = (int *) malloc(block * sizeof(int));

    if (rank == 0) {
        global = (int *) malloc(ARRAY_SIZE * sizeof(int));
        for (int i = 0; i < ARRAY_SIZE; i++)
            global[i] = ARRAY_SIZE - i;  // worst case
        printf("Unsorted:\n");
        for (int i = 0; i < ARRAY_SIZE; i++)
            printf("%d ", global[i]);
        printf("\n");
    }

    MPI_Scatter(global, block, MPI_INT, local, block, MPI_INT, 0, MPI_COMM_WORLD);

    // local bubble sort
    bubble_sort(block, local);

    int *recv = (int *) malloc(block * sizeof(int));

    // odd-even transposition sort
    for (int phase = 0; phase < nprocs; phase++) {
        int partner;

        if (phase % 2 == 0) {        // even phase
            partner = (rank % 2 == 0) ? rank + 1 : rank - 1;
        } else {                     // odd phase
            partner = (rank % 2 == 0) ? rank - 1 : rank + 1;
        }

        if (partner < 0 || partner >= nprocs) {
            // no partner → skip
        } else {
            MPI_Sendrecv(local, block, MPI_INT, partner, 0,
                          recv,  block, MPI_INT, partner, 0,
                          MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            if (rank < partner)
                merge_low(local, recv, block);
            else
                merge_high(local, recv, block);
        }
    }

    MPI_Gather(local, block, MPI_INT, global, block, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("\nSorted:\n");
        for (int i = 0; i < ARRAY_SIZE; i++)
            printf("%d ", global[i]);
        printf("\n");
    }

    free(local);
    free(recv);
    if (rank == 0) free(global);

    MPI_Finalize();
    return 0;
}
