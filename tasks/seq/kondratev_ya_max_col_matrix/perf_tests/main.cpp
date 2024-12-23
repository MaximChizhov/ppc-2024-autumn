// Copyright 2023 Nesterov Alexander
#include <gtest/gtest.h>

#include <random>
#include <vector>

#include "core/perf/include/perf.hpp"
#include "seq/kondratev_ya_max_col_matrix/include/ops_seq.hpp"

std::vector<std::vector<int32_t>> getRandomMatrix(uint32_t row, uint32_t col) {
  int32_t low = -200;
  int32_t high = 200;

  std::random_device dev;
  std::mt19937 gen(dev());
  std::vector<std::vector<int32_t>> mtrx(row, std::vector<int32_t>(col));
  for (uint32_t i = 0; i < row; i++) {
    for (uint32_t j = 0; j < col; j++) {
      mtrx[i][j] = low + gen() % (high - low + 1);
    }
  }
  return mtrx;
}

void insertRefValue(std::vector<std::vector<int32_t>>& mtrx, int32_t ref) {
  std::random_device dev;
  std::mt19937 gen(dev());

  uint32_t ind;
  uint32_t row = mtrx.size();
  uint32_t col = mtrx[0].size();

  for (uint32_t j = 0; j < col; j++) {
    ind = gen() % row;
    mtrx[ind][j] = ref;
  }
}

void fillTaskData(std::shared_ptr<ppc::core::TaskData>& taskData, uint32_t row, uint32_t col, auto& mtrx, auto& res) {
  for (auto& mtrxRow : mtrx) taskData->inputs.emplace_back(reinterpret_cast<uint8_t*>(mtrxRow.data()));
  taskData->inputs_count.emplace_back(row);
  taskData->inputs_count.emplace_back(col);
  taskData->outputs.emplace_back(reinterpret_cast<uint8_t*>(res.data()));
  taskData->outputs_count.emplace_back(res.size());
}

TEST(kondratev_ya_max_col_matrix_seq, test_pipeline_run) {
  uint32_t row = 6000;
  uint32_t col = 6000;
  int32_t ref_val = INT_MAX;

  std::vector<int32_t> res(col);
  std::vector<int32_t> ref(col, ref_val);
  std::vector<std::vector<int32_t>> mtrx = getRandomMatrix(row, col);
  insertRefValue(mtrx, ref_val);

  auto taskDataSeq = std::make_shared<ppc::core::TaskData>();
  fillTaskData(taskDataSeq, row, col, mtrx, res);

  auto testTaskSequential = std::make_shared<kondratev_ya_max_col_matrix_seq::TestTaskSequential>(taskDataSeq);

  // Create Perf attributes
  auto perfAttr = std::make_shared<ppc::core::PerfAttr>();
  perfAttr->num_running = 10;  // Set the number of runs as needed
  const auto t0 = std::chrono::high_resolution_clock::now();
  perfAttr->current_timer = [&] {
    auto current_time_point = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time_point - t0).count();
    return static_cast<double>(duration) * 1e-9;
  };

  // Create and init perf results
  auto perfResults = std::make_shared<ppc::core::PerfResults>();

  // Create Perf analyzer
  auto perfAnalyzer = std::make_shared<ppc::core::Perf>(testTaskSequential);
  perfAnalyzer->pipeline_run(perfAttr, perfResults);
  ppc::core::Perf::print_perf_statistic(perfResults);

  for (uint32_t i = 0; i < res.size(); i++) ASSERT_EQ(res[i], ref[i]);
}

TEST(kondratev_ya_max_col_matrix_seq, test_task_run) {
  uint32_t row = 6000;
  uint32_t col = 6000;
  int32_t ref_val = INT_MAX;

  std::vector<int32_t> res(col);
  std::vector<int32_t> ref(col, ref_val);
  std::vector<std::vector<int32_t>> mtrx = getRandomMatrix(row, col);
  insertRefValue(mtrx, ref_val);

  auto taskDataSeq = std::make_shared<ppc::core::TaskData>();
  fillTaskData(taskDataSeq, row, col, mtrx, res);

  auto testTaskSequential = std::make_shared<kondratev_ya_max_col_matrix_seq::TestTaskSequential>(taskDataSeq);

  // Create Perf attributes
  auto perfAttr = std::make_shared<ppc::core::PerfAttr>();
  perfAttr->num_running = 10;
  const auto t0 = std::chrono::high_resolution_clock::now();
  perfAttr->current_timer = [&] {
    auto current_time_point = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time_point - t0).count();
    return static_cast<double>(duration) * 1e-9;
  };

  // Create and init perf results
  auto perfResults = std::make_shared<ppc::core::PerfResults>();

  // Create Perf analyzer
  auto perfAnalyzer = std::make_shared<ppc::core::Perf>(testTaskSequential);
  perfAnalyzer->task_run(perfAttr, perfResults);
  ppc::core::Perf::print_perf_statistic(perfResults);

  for (uint32_t i = 0; i < res.size(); i++) ASSERT_EQ(res[i], ref[i]);
}
