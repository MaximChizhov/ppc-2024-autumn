// Copyright 2024 Nesterov Alexander
#include "seq/chizhov_m_algorithm_dijkstra/include/ops_seq.hpp"

#include <algorithm>
#include <vector>

const int INF = std::numeric_limits<int>::max();

bool chizhov_m_dijkstra_seq::TestTaskSequential::pre_processing() {
  internal_order_test();
  // Init value for input and output
  size = taskData->inputs_count[0];
  st = taskData->inputs_count[1];

  input_ = std::vector<std::vector<int>>(size, std::vector<int>(size));
  for (int i = 0; i < size; i++) {
    auto* tmp_ptr = reinterpret_cast<int*>(taskData->inputs[i]);
    for (int j = 0; j < size; j++) {
      input_[i][j] = tmp_ptr[j];
    }
  }

  res_ = std::vector<int>(size, 0);
  return true;
}

bool chizhov_m_dijkstra_seq::TestTaskSequential::validation() {
  internal_order_test();

  if (taskData->inputs.empty() || taskData->inputs.size() < 2 || taskData->inputs[0] == nullptr ||
      taskData->inputs[1] == nullptr) {
    return false;
  }

  if (taskData->inputs_count.size() < 2 || taskData->inputs_count[0] <= 0) {
    return false;
  }

  for (unsigned int i = 0; i < taskData->inputs_count[0]; i++) {
    auto* tmp_ptr = reinterpret_cast<int*>(taskData->inputs[i]);
    if (!std::all_of(tmp_ptr, tmp_ptr + taskData->inputs_count[0], [](int val) { return val >= 0; })) {
      return false;
    }
  }

  if (taskData->inputs_count[1] < 0 || taskData->inputs_count[1] >= taskData->inputs_count[0]) {
    return false;
  }

  if (taskData->outputs.empty() || taskData->outputs[0] == nullptr || taskData->outputs.size() != 1 ||
      taskData->outputs_count[0] != taskData->inputs_count[0]) {
    return false;
  }

  return true;
}

void chizhov_m_dijkstra_seq::convertToCRS(const std::vector<std::vector<int>>& w, std::vector<int>& values,
                                          std::vector<int>& colIndex, std::vector<int>& rowPtr) {
  int n = w.size();
  rowPtr.resize(n + 1);
  int nnz = 0;

  for (int i = 0; i < n; i++) {
    rowPtr[i] = nnz;
    for (int j = 0; j < n; j++) {
      if (w[i][j] != 0) {
        values.emplace_back(w[i][j]);
        colIndex.emplace_back(j);
        nnz++;
      }
    }
  }
  rowPtr[n] = nnz;
}

bool chizhov_m_dijkstra_seq::TestTaskSequential::run() {
  internal_order_test();
  std::vector<int> values;
  std::vector<int> colIndex;
  std::vector<int> rowPtr;
  convertToCRS(input_, values, colIndex, rowPtr);

  std::vector<bool> visited(size, false);
  std::vector<int> D(size, INF);
  D[st] = 0;

  for (int i = 0; i < size; i++) {
    int min = INF;
    int index = -1;
    for (int j = 0; j < size; j++) {
      if (!visited[j] && D[j] < min) {
        min = D[j];
        index = j;
      }
    }

    if (index == -1) break;

    int u = index;
    visited[u] = true;

    for (int j = rowPtr[u]; j < rowPtr[u + 1]; j++) {
      int v = colIndex[j];
      int weight = values[j];

      if (!visited[v] && D[u] != INF && (D[u] + weight < D[v])) {
        D[v] = D[u] + weight;
      }
    }
  }

  res_ = D;

  return true;
}

bool chizhov_m_dijkstra_seq::TestTaskSequential::post_processing() {
  internal_order_test();
  std::copy(res_.begin(), res_.end(), reinterpret_cast<int*>(taskData->outputs[0]));
  return true;
}