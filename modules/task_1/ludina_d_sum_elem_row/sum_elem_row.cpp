// Copyright 2020 Ludina Daria
#include "../../../modules/task_1/ludina_d_sum_elem_row/sum_elem_row.h"
#include <vector>

std::vector<int> createRandomMatrix(int rows, int cols) {
  std::mt19937 gen;
  gen.seed(static_cast<unsigned int>(time(0)));
  int m_size = rows * cols;
  std::vector<int> vec(m_size);
  for (int i = 0; i < m_size; i++) {
    vec[i] = gen() % 100;
  }
  return vec;
}

std::vector<int> getSequentialOperations(std::vector<int> matrix, int rows, int cols) {
  std::vector<int> sum_elem(rows);
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      sum_elem[i] += matrix[i * cols + j];
    }
  }
  return sum_elem;
}

std::vector<int> getParallelOperations(std::vector<int> global_matrix, int rows, int cols) {
  int size, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  const int delta = rows / size;
  const int rest = rows % size;
  if (rank == 0) {
    for (int proc = 1; proc < size; proc++) {
      MPI_Send(&global_matrix[0] + proc * delta * cols + rest * cols, delta * cols, MPI_INT, proc, 0, MPI_COMM_WORLD);
    }
  }
  std::vector<int> local_matrix(delta * cols);
  if (rank == 0) {
    local_matrix.resize((delta + rest) * cols);
    local_matrix = std::vector<int>(global_matrix.begin(), global_matrix.begin() + (delta + rest) * cols);
  } else {
    MPI_Status status;
    MPI_Recv(&local_matrix[0], delta * cols, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
  }
  int local_row = delta;
  if (rank == 0)
    local_row = delta + rest;
  std::vector<int> global_sum(rows);
  std::vector<int> local_sum(local_row);
  local_sum = getSequentialOperations(local_matrix, local_row, cols);
  if (rank == 0) {
    for (int i = 0; i < local_row; i++)
      global_sum[i] = local_sum[i];
    for (int proc = 1; proc < size; proc++) {
      MPI_Status status;
      MPI_Recv(&global_sum[0] + delta * proc + rest, delta, MPI_INT, proc, 0, MPI_COMM_WORLD, &status);
    }
  } else {
    MPI_Send(&local_sum[0], local_row, MPI_INT, 0, 0, MPI_COMM_WORLD);
  }
  return global_sum;
}
