/*
 * Copyright (c) 2004-2006 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2006      Cisco Systems, Inc.  All rights reserved.
 *
 * Sample MPI "hello comm" application in C
 */

#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "mpi-ext.h"
#define SIZE 1024

static void verbose_errhandler(MPI_Comm* comm, int* err, int len) {
	char errstr[MPI_MAX_ERROR_STRING];

    	int rank,size,nf;
	MPI_Group group_f;
	MPI_Comm_rank(*comm, &rank);
	MPI_Comm_size(*comm, &size);
	MPI_Error_string( *err, errstr, &len );
	MPIX_Comm_failure_ack(*comm);
	MPIX_Comm_failure_get_acked(*comm, &group_f);
	MPI_Group_rank(group_f, &nf);
	//printf("Rank %d / %d: Notified of error %s. %d found dead.\n", rank, size, errstr, nf);
}

int main(int argc, char* argv[])
{
    int rank, size, len;
    char version[MPI_MAX_LIBRARY_VERSION_STRING];
    double * buffer = (double *) malloc(sizeof(double) * SIZE);
    //double buffer[SIZE];
    MPI_Init(&argc, &argv);
    MPI_Comm comm = MPI_COMM_WORLD;

    MPI_Errhandler errh;
    MPI_Comm_create_errhandler(&verbose_errhandler, &errh);
    MPI_Comm_set_errhandler(comm, errh);

    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    MPI_Get_library_version(version, &len);
  
    double starttime, endtime;

    for (int i=0; i<10; i++) {
	    MPI_Barrier(comm);
	   if (rank == 0)
		   printf("Star benchmarking with size = %d\n", size);
	    starttime = MPI_Wtime();
		// Kill process
	    if ((i == 3) && (rank == 1)) {
		exit(0);
    	    }
	    int rc = MPI_Bcast(buffer, SIZE, MPI_DOUBLE, 0, comm);
 	    if (rc != MPI_SUCCESS) {
		    //printf(" process %d : go repair ...\n", rank);
		    MPIX_Comm_revoke(comm);
		    MPI_Comm shrinked;
		    MPIX_Comm_shrink(comm, &shrinked);
		    int size;
		    MPI_Comm_size(shrinked, &size);
		
		    comm = shrinked;
		    MPI_Comm_set_errhandler(comm, errh);
		    MPI_Comm_rank(comm, &rank);
	
		    if (rank == 0)
			    printf("New size of comm = %d\n",size);
		//MPIX_Comm_revoke(comm);
		   MPI_Bcast(buffer, SIZE, MPI_DOUBLE, 0, comm);
	    }
	    MPI_Barrier(comm);
	    MPI_Comm_size(comm, &size);
	    printf("End benchmarking with size = %d\n", size);
	    endtime = MPI_Wtime();
	    if (rank == 0)
		    printf("Bcast %d took %lf\n", i, (endtime-starttime));

    }
    free(buffer);
    MPI_Finalize();

    return 0;
}
