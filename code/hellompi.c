#include <stdio.h>
#include <sys/time.h>
#include <mpi.h>

int main(int argc, char **argv) {
        int me, nprocs, namelen;
        char processor_name[MPI_MAX_PROCESSOR_NAME];
        struct timeval tv1, tv2;

        gettimeofday(&tv1, NULL);

        MPI_Init(&argc, &argv);
        MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
        MPI_Comm_rank(MPI_COMM_WORLD, &me);
        MPI_Get_processor_name(processor_name, &namelen);

        printf("Hello World! I'm  process %d of %d on %s\n", me, nprocs, processor_name);

        MPI_Finalize();

        gettimeofday(&tv2, NULL);
        printf("Total time: %f\n", (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec));

        return 0;
}
