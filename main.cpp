#include <iostream>
#include <algorithm>
#include "mpi.h"
#include "main.h"

#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION "0.1.0"
#endif

uint64_t proc_freq, tprof[LIM_R][LIM_C], prof[LIM_R];

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

std::vector<char> string_to_char(const std::vector<std::string> &strings) {
  std::vector<char> cstrings;
  cstrings.reserve(strings.size());
  for (const std::string &s: strings) {
    for (size_t i = 0; i < strlen(s.c_str()); ++i) {
      cstrings.push_back(s.c_str()[i]);
    }
  }
  return cstrings;
}

void merge_results(const std::string &input, int rank, int world_size) {
  std::cout << "Merging results..." << std::endl;

  std::ifstream file(input, std::ios::in);

  std::vector<std::string> final_outputs;

  const std::string sq = "@SQ";

  std::string line;
  while (std::getline(file, line)) {
    if (line.rfind(sq) == 0) {
      final_outputs.push_back(line);
    }
  }

  std::vector<char> char_vector = string_to_char(final_outputs);

  std::vector<int32_t> sizes;
  sizes.reserve(world_size);

  int32_t this_worker_size = char_vector.size();
  MPI_Gather(&this_worker_size, 1, MPI_INT32_T, &sizes, world_size, MPI_INT32_T, 0, MPI_COMM_WORLD);

  int32_t total_chars = 0;
  std::vector<char> char_vectors_recv(total_chars);
  std::vector<int> displs(world_size + 1);
  displs.push_back(0);

  for (int32_t s:sizes) {
    total_chars += s;
    displs.push_back(s);
  }

  MPI_Gatherv(char_vector.data(),
              char_vector.size(),
              MPI_CHAR,
              char_vector.data(),
              reinterpret_cast<const int *>(total_chars),
              displs.data(),
              MPI_CHAR,
              0,
              MPI_COMM_WORLD);

  if (rank == 0) {
    std::string s(char_vectors_recv.begin(), char_vectors_recv.end());
    std::cout << "RESULT IN WORKER 0" << std::endl;
    std::cout << s << std::endl;
  }
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

    // wait for worker 0 to finish splitting
    MPI_Barrier(MPI_COMM_WORLD);

    std::cout << "Calling BWA indexing with partition " << argv[2] << std::endl;

    ret = bwa_index(argc - 1, argv + 1);
    //todo add time taken
    std::cout << "Indexing completed by " << rank << " with return code " << ret << std::endl;
  } else if (strcmp(argv[1], "mem") == 0) {
    tprof[MEM][0] = __rdtsc();

    std::string new_src = std::string(argv[4]) + ".split." + std::to_string(rank);
    argv[4] = (char *) new_src.c_str();
    std::cout << "New Source for Worker " << rank << " : " << new_src << std::endl;

    std::string new_dest = std::string(argv[argc - 1]) + "." + std::to_string(rank);
    argv[argc - 1] = (char *) new_dest.c_str();
    std::cout << "New Destination for Worker " << rank << " : " << new_dest << std::endl;

    kstring_t pg = {0, 0, 0};
    extern char *bwa_pg;

    ksprintf(&pg, "@PG\tID:bwa\tPN:bwa\tVN:%s\tCL:%s", PACKAGE_VERSION, argv[0]);
    for (int i = 1; i < argc; ++i) ksprintf(&pg, " %s", argv[i]);
    ksprintf(&pg, "\n");
    bwa_pg = pg.s;
    ret = main_mem(argc - 1, argv + 1);
    free(bwa_pg);

    MPI_Barrier(MPI_COMM_WORLD);

    // now read the output again and do merging

    merge_results(new_dest, rank, world_size);
  }

  MPI_Finalize();
  return 0;
}
