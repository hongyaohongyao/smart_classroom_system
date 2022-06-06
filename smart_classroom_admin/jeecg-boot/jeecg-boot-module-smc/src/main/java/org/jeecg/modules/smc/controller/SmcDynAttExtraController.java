package org.jeecg.modules.smc.controller;

import io.swagger.annotations.Api;
import io.swagger.annotations.ApiOperation;
import lombok.extern.slf4j.Slf4j;
import org.jeecg.common.api.vo.Result;
import org.jeecg.common.aspect.annotation.AutoLog;
import org.jeecg.common.system.base.controller.JeecgController;
import org.jeecg.modules.smc.entity.SmcDynAtt;
import org.jeecg.modules.smc.service.ISmcDynAttService;
import org.jeecg.modules.smc.service.ISmcDynAttTaskService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

/**
 * @Description: 在线点名附加功能
 * @Author: hongyaohongyao
 * @Date: 2022-03-11
 * @Version: V1.0
 */
@Api(tags = "在线点名附加功能")
@RestController
@RequestMapping("/smc/smcDynAttExtra")
@Slf4j
public class SmcDynAttExtraController extends JeecgController<SmcDynAtt, ISmcDynAttService> {

    @Autowired
    private ISmcDynAttTaskService smcDynAttTaskService;


    /**
     * 通过id停止任务
     *
     * @param id
     * @return
     */
    @AutoLog(value = "在线点名-通过id停止任务")
    @ApiOperation(value = "在线点名-通过id停止任务", notes = "在线点名-通过id停止任务")
    @DeleteMapping(value = "/stop")
    public Result<?> stopById(@RequestParam(name = "id", required = true) String id) {
        smcDynAttTaskService.stopById(id);
        return Result.OK("停止成功!");
    }

    /**
     * 通过id停止任务
     *
     * @param id
     * @return
     */
    @AutoLog(value = "在线点名-通过id延长任务")
    @ApiOperation(value = "在线点名-通过id延长任务", notes = "在线点名-通过id延长任务")
    @GetMapping(value = "/alive")
    public Result<?> aliveById(@RequestParam(name = "id", required = true) String id) {
        boolean result = smcDynAttTaskService.aliveById(id);
        return result ? Result.OK("续命成功!") : Result.error("停止续命");
    }


}
