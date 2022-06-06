#ifndef SMART_CLASSROOM_CPP_SMART_CLASSROOM_HPP
#define SMART_CLASSROOM_CPP_SMART_CLASSROOM_HPP

#include <string>
#include <atomic>
#include <common/ilogger.hpp>
#include <dynamic_attendance/dynamic_attendance.hpp>

namespace smc::smc_app {
    constexpr int INPUT_TYPE_LOCAL = 0;
    constexpr int INPUT_TYPE_WEB = 1;
    constexpr int INPUT_TYPE_CAP = -1;
    constexpr int INPUT_TYPE_DEFAULT = INPUT_TYPE_LOCAL;

    constexpr int OUTPUT_TYPE_SHOW = 0;
    constexpr int OUTPUT_TYPE_RTMP = 1;
    constexpr int OUTPUT_TYPE_DEFAULT = OUTPUT_TYPE_SHOW;

    constexpr int FILE_SYS_TYPE_LOCAL = 0;
    constexpr int FILE_SYS_TYPE_FTP = 1;
    constexpr int FILE_SYS_TYPE_MINIO = 2;
    constexpr int FILE_SYS_TYPE_DEFAULT = FILE_SYS_TYPE_LOCAL;

    constexpr int WORKING_STATE_RUNNING = 0;
    constexpr int WORKING_STATE_STOPPING = 1;

    int cheatingDetection(std::atomic_ullong &end_time,
                          const std::string &source = "front_cheat.mp4",
                          const std::string &taskName = "default",
                          int input_type = INPUT_TYPE_DEFAULT,
                          int output_type = OUTPUT_TYPE_DEFAULT,
                          int file_sys_type = FILE_SYS_TYPE_DEFAULT,
                          const std::string &rtmpServer = SMC_RTMP_SERVER,
                          const std::string &fileSysDir = ".");

    int dynamicAttendance(std::atomic_ullong &end_time, dyn_att::Info &info,
                          const std::string &source = "dynamic_attendance.mp4",
                          const std::string &taskName = "default",
                          int input_type = INPUT_TYPE_DEFAULT,
                          int output_type = OUTPUT_TYPE_DEFAULT,
                          int file_sys_type = FILE_SYS_TYPE_DEFAULT,
                          const std::string &dstUrl = SMC_RTMP_SERVER, const std::string &fileSysDir = "."
    );

    int serverMain(int argc, char **argv);

    int testDynamicAttendance();

}

#endif //SMART_CLASSROOM_CPP_SMART_CLASSROOM_HPP
