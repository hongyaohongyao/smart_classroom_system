#include <smc_svr/smc_svr.hpp>
#include <common/ilogger.hpp>

int smc::svr::svr_main(int argc, char **argv) {
    INFO("Server running on %s:%d", SMC_SERVER_HOST, SMC_SERVER_PORT);
    app().setClientMaxBodySize(1024 * 1024 * 1024);
    app().addListener(SMC_SERVER_HOST, SMC_SERVER_PORT).run();
    return 0;
}