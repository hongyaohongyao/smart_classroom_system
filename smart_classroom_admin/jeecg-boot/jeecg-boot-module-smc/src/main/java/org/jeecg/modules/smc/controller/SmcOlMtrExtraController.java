package org.jeecg.modules.smc.controller;

import io.swagger.annotations.Api;
import io.swagger.annotations.ApiOperation;
import lombok.extern.slf4j.Slf4j;
import org.jeecg.common.api.vo.Result;
import org.jeecg.common.aspect.annotation.AutoLog;
import org.jeecg.modules.smc.service.ISmcOlMtrTaskService;
import org.jeecg.utils.ResultUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

/**
 * @Description: 智慧教室在线监考附加功能
 * @Author: hongyaohongyao
 * @Date: 2022-04-16
 * @Version: V1.0
 */
@Api(tags = "智慧教室在线监考附加功能")
@RestController
@RequestMapping("/smc/smcOlMtrExtra")
@Slf4j
public class SmcOlMtrExtraController {
    @Autowired
    private ISmcOlMtrTaskService smcOlMtrTaskService;


    /**
     * 通过id进入下一步
     *
     * @param id
     * @return
     */
    @AutoLog(value = "智慧教室在线监考-通过id进入下一步")
    @ApiOperation(value = "智慧教室在线监考-通过id进入下一步", notes = "智慧教室在线监考-通过id进入下一步")
    @GetMapping(value = "/nextStep")
    public Result<?> delete(@RequestParam(name = "id", required = true) String id) {
        return ResultUtils.msgResult(smcOlMtrTaskService.nextState(id), "步骤完成");
    }
}
