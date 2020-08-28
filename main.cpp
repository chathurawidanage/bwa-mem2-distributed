#include <iostream>
#include "mpi.h"

void split_file_with_overlaps(const std::string &path) {
  std::cout << "Starting worker " << rank << std::endl;
}

int main(int argc, char *argv[]) {
  MPI_Init(&argc, &argv);

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  std::cout << "Starting worker " << rank << std::endl;

  if (rank == 0) {
    split_file_with_overlaps(argv[0]);
  }

  MPI_Finalize();
  return 0;
}
