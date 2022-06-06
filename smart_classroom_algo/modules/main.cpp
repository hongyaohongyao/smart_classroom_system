
#include <smc_svr/smc_svr.hpp>

int test_main();

int main(int argc, char **argv) {
  //    return smc::smc_app::testDynamicAttendance();
  //    return smc::smc_app::serverMain(argc, argv);
  //  return test_main();
  return smc::svr::svr_main(argc, argv);
}
