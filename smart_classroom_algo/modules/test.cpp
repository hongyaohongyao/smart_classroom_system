#include "common/models_utils.hpp"

int test_main() {
//  smc::models_utils::createArcface();
  std::string file = "arcface_iresnet50.FP32.trtmodel";
  TRT::set_device(0);
  auto engine = TRT::load_infer(file);
  auto input = engine->input();
  input->resize_single_dim(0, 1);
  // warm up
  for (int i = 0; i < 10; i++) {
    engine->forward(true);
  }
  int test_num = 100;
  auto time_begin = iLogger::timestamp_now_float();
  for (int i = 0; i < test_num; i++) {
    engine->forward(false);
  }
  engine->synchronize();
  float average_time = (iLogger::timestamp_now_float() - time_begin) / test_num;
  INFO("infer %f ms, %f fps", average_time, 1000 / average_time);
  return 0;
}