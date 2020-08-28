#include <iostream>
#include <fstream>
#include <algorithm>
#include "mpi.h"

void split_file_with_overlaps(const std::string &path, int splits, int overlap_bytes) {
  std::cout << "Splitting reference file " << path << std::endl;
  std::ifstream file(path, std::ios::in);

  std::streampos file_size = file.tellg();
  file.seekg(0, std::ios::end);
  file_size = file.tellg() - file_size;
  file.seekg(0, std::ios::beg);

  std::cout << "File Size : " << file_size << std::endl;

  std::string header;
  std::string str;
  std::getline(file, header); // header
  std::getline(file, str); // normal line
  auto line_size = ((str.size() + 1) * sizeof(char)); // +1 for line break

  auto lines_count_estimate = file_size / line_size;
  auto overlap_lines = overlap_bytes / line_size;

  auto lines_per_worker = lines_count_estimate / splits;

  file.seekg(0, std::ios::beg);

  std::getline(file, str); // skip header

  // todo implement more efficient block copying

  std::streampos rollback_point = file.tellg();
  for (int32_t split = 0; split < splits; split++) {
    file.seekg(rollback_point);
    std::ofstream out_split(path + ".split." + std::to_string(split));
    out_split << header << std::endl;
    uint64_t lines_written = 0;
    while (std::getline(file, str)) {
      out_split << str << std::endl;
      lines_written++;
      if (lines_written == lines_per_worker - overlap_lines) {
        rollback_point = file.tellg();
      } else if (lines_written == lines_per_worker + overlap_lines) {
        break;
      }
    }
    std::cout << "Writing split " << split << " of size " << out_split.tellp() << std::endl;
    out_split.close();
  }

}

int main(int argc, char *argv[]) {
  MPI_Init(&argc, &argv);

  int rank, world_size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  std::cout << "Starting worker " << rank << std::endl;

  if (rank == 0) {
    split_file_with_overlaps(argv[1], world_size, 1024 * 1024);
  }

  MPI_Finalize();
  return 0;
}
