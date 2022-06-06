//
//package org.jeecg.modules.demo.xxljob;
//
//import com.xxl.job.core.biz.model.ReturnT;
//import com.xxl.job.core.handler.annotation.XxlJob;
//import lombok.extern.slf4j.Slf4j;
//import org.springframework.stereotype.Component;
//
//
///**
// * xxl-job定时任务测试
// */
//@Component
//@Slf4j
//public class TestJobHandler {
//
//
//    /**
//     * 简单任务
//     *
//     * @param params
//     * @return
//     */
//
//    @XxlJob(value = "testJob")
//    public ReturnT<String> demoJobHandler(String params) {
//        log.info("我是demo服务里的定时任务testJob,我执行了...............................");
//        return ReturnT.SUCCESS;
//    }
//
//    public void init() {
//        log.info("init");
//    }
//
//    public void destroy() {
//        log.info("destory");
//    }
//
//}
//
