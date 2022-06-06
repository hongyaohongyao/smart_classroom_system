package org.jeecg.modules.smc.controller;

import com.alibaba.fastjson.JSONObject;
import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import io.swagger.annotations.Api;
import io.swagger.annotations.ApiOperation;
import lombok.extern.slf4j.Slf4j;
import org.jeecg.common.api.vo.Result;
import org.jeecg.common.aspect.annotation.AutoLog;
import org.jeecg.modules.smc.entity.SmcAiServer;
import org.jeecg.modules.smc.entity.SmcClass;
import org.jeecg.modules.smc.entity.SmcOlMtr;
import org.jeecg.modules.smc.entity.SmcVideoSrc;
import org.jeecg.modules.smc.service.*;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

/**
 * @Description: 首页信息
 * @Author: hongyaohongyao
 * @Date: 2022-05-11
 * @Version: V1.0
 */
@Api(tags = "首页信息")
@RestController
@RequestMapping("/smc/home")
@Slf4j
public class HomeController {
    @Autowired
    ISmcAiServerService smcAiServerService;
    @Autowired
    ISmcFaceRegService smcFaceRegService;
    @Autowired
    ISmcOlMtrService smcOlMtrService;
    @Autowired
    ISmcClassService smcClassService;
    @Autowired
    ISmcVideoSrcService smcVideoSrcService;

    /**
     * 获取考生人数信息
     *
     * @return
     */
    @AutoLog(value = "智慧教室首页-考生数据")
    @ApiOperation(value = "智慧教室首页-考生数据", notes = "智慧教室首页-考生数据")
    @GetMapping(value = "/students")
    public Result<?> getStudents() {
        JSONObject result = new JSONObject();
        result.put("total", smcFaceRegService.count());
        return Result.OK("ok", result);
    }

    /**
     * 获取考场信息
     *
     * @return
     */
    @AutoLog(value = "智慧教室首页-考场信息")
    @ApiOperation(value = "智慧教室首页-考场信息", notes = "智慧教室首页-考场信息")
    @GetMapping(value = "/classroom")
    public Result<?> getClassroom() {
        JSONObject result = new JSONObject();
        result.put("total", smcClassService.count());
        result.put("videos", smcVideoSrcService.count());
        return Result.OK("ok", result);
    }

    /**
     * 获取考生人数信息
     *
     * @return
     */
    @AutoLog(value = "智慧教室首页-考试场数")
    @ApiOperation(value = "智慧教室首页-考试场数", notes = "智慧教室首页-考试场数")
    @GetMapping(value = "/olMtrs")
    public Result<?> getOlMtrs() {
        JSONObject result = new JSONObject();
        result.put("total", smcOlMtrService.count());
        QueryWrapper<SmcOlMtr> queryWrapper = new QueryWrapper<>();
        queryWrapper.le("task_status", 3);
        result.put("finish", smcOlMtrService.count(queryWrapper));
        return Result.OK("ok", result);
    }

    /**
     * 获取ai服务器信息
     *
     * @return
     */
    @AutoLog(value = "智慧教室首页-ai服务器数据")
    @ApiOperation(value = "智慧教室首页-ai服务器数据", notes = "智慧教室首页-ai服务器数据")
    @GetMapping(value = "/aiServer")
    public Result<?> getAiServer() {
        JSONObject result = new JSONObject();
        result.put("total", smcAiServerService.count());
        QueryWrapper<SmcAiServer> queryWrapper = new QueryWrapper<>();
        queryWrapper.eq("server_state", 0);
        result.put("available", smcAiServerService.count(queryWrapper));
        return Result.OK("ok", result);
    }

    /**
     * 获取指定考试信息
     *
     * @return
     */
    @AutoLog(value = "智慧教室首页-指定考试信息")
    @ApiOperation(value = "智慧教室首页-指定考试信息", notes = "智慧教室首页-指定考试信息")
    @GetMapping(value = "/olMtrInfo")
    public Result<?> getOlMtrInfo(@RequestParam(name = "id", required = true) String id) {
        JSONObject result = new JSONObject();
        SmcOlMtr smcOlMtr = smcOlMtrService.getById(id);
        SmcClass smcClass = smcClassService.getById(smcOlMtr.getClassId());
        SmcVideoSrc videoSrc = smcVideoSrcService.getById(smcOlMtr.getVideoSrcId());
        result.put("classroomName", smcClass.getClassName());
        result.put("videoName", videoSrc.getVideoName());
        return Result.OK("ok", result);
    }
}
