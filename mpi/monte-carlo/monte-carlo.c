#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int inside(double x, double y, int r){
  if(((x*x) + (y*y)) < (r*r)){
    return 1;
  }
  return 0;
}

int calc_circle(int npoints){
  int circle_count;
  double x, y;
  int r = 1;
  for(int i=0; i < npoints; i++){
    x = (random() % 101)/100.0;
    y = (random() % 101)/100.0;
    if(inside(x,y,r)){
      circle_count++;
    }
  }
  return circle_count;
}

int main(int argc, char** argv) {
    srand( (unsigned)time(NULL) );
    int size, rank;
    double pi;
    int npoints, num, circle_count;
    int *globaldata=NULL;

    // Initialize the MPI environment
    MPI_Init(&argc,&argv);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //printf("Rank %d de um total de %d\n", rank, size);
    if(rank == 0){
      npoints = 10000; //200000000
      num = npoints/size;
      globaldata = malloc(size * sizeof(int));

    }


    MPI_Bcast(&num, 1, MPI_INT,0, MPI_COMM_WORLD);
    printf("RANK: %d\t POINTS: %d\n", rank, num);

    circle_count = calc_circle(num);

    MPI_Gather(&circle_count, 1, MPI_INT, globaldata, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if(rank == 0){
      int sum=0;
      for(int i=0; i < size; i++){
        //printf("%d\n", globaldata[i]); Ta certo isso ?
        sum =  sum + globaldata[i];
      }
      pi = ((double)4*sum)/(double)npoints;
      printf("PI: %.2f\n",pi);
      free(globaldata);
    }

    // Finalize the MPI environment.
    MPI_Finalize();
}
