#include <iostream>
#include <fstream>
#include <algorithm>
#include "mpi.h"
#include "main.h"

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
  int64_t line_size = ((str.size() + 1) * sizeof(char)); // +1 for line break

  int64_t lines_count_estimate = file_size / line_size;
  int64_t overlap_lines = overlap_bytes / line_size;

  int64_t lines_per_worker = lines_count_estimate / splits;

  file.seekg(0, std::ios::beg);

  std::getline(file, str); // skip header

  // todo implement more efficient block copying

  std::streampos rollback_point = file.tellg();
  for (int32_t split = 0; split < splits; split++) {
    file.seekg(rollback_point);
    std::ofstream out_split(path + ".split." + std::to_string(split));
    out_split << header << std::endl;
    int64_t lines_written = 0;
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

int usage() {
  fprintf(stderr, "Usage: bwa-mem2 <command> <arguments>\n");
  fprintf(stderr, "Commands:\n");
  fprintf(stderr, "  index         create index\n");
  fprintf(stderr, "  mem           alignment\n");
  fprintf(stderr, "  version       print version number\n");
  return 1;
}

int main(int argc, char *argv[]) {
  MPI_Init(&argc, &argv);

  int rank, world_size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  std::cout << "Starting worker " << rank << std::endl;

  int ret = -1;
  if (argc < 2) return usage();

  if (strcmp(argv[1], "index") == 0) {
    if (rank == 0) {
      split_file_with_overlaps(argv[2], world_size, 1024 * 1024);
    }
    std::string new_src = std::string(argv[2]) + ".split." + std::to_string(rank);

    argv[2] = (char *) new_src.c_str();

    std::cout << "Calling BWA indexing with partition " << argv[2] << std::endl;
    // wait for worker 0 to finish splitting
    MPI_Barrier(MPI_COMM_WORLD);
    ret = bwa_index(argc - 1, argv + 1);
    //todo add time taken
    return ret;
  }

  MPI_Finalize();
  return 0;
}
