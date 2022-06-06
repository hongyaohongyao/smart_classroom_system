package org.jeecg.modules.smc.controller;

import io.swagger.annotations.Api;
import io.swagger.annotations.ApiOperation;
import lombok.extern.slf4j.Slf4j;
import org.jeecg.common.api.vo.Result;
import org.jeecg.common.aspect.annotation.AutoLog;
import org.jeecg.modules.smc.service.ISmcAiServerManagerService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.util.Arrays;
import java.util.Collection;

/**
 * @Description: 智慧教室ai服务器连接控制api
 * @Author: hongyaohongyao
 * @Date: 2022-02-25
 * @Version: V1.0
 */
@Api(tags = "智慧教室ai服务器连接控制api")
@RestController
@RequestMapping("/smc/smcAiServerExtra")
@Slf4j
public class SmcAiServerExtraController {
    @Autowired
    ISmcAiServerManagerService smcAiServerManagerService;

    /**
     * 通过id删除
     *
     * @param id
     * @return
     */
    @AutoLog(value = "智慧教室ai服务器-重启")
    @ApiOperation(value = "智慧教室ai服务器-通过id重启", notes = "智慧教室ai服务器-通过id重启")
    @GetMapping(value = "/reconnect")
    public Result<?> reconnect(@RequestParam(name = "id", required = true) String id) {
        try {
            if (smcAiServerManagerService.reconnect(id)) {
                return Result.OK("重连成功!");
            } else {
                return Result.error("重连失败!请检查服务器是否开启");
            }
        } finally {
            smcAiServerManagerService.refreshAiServers();
        }
    }

    /**
     * 批量删除
     *
     * @param ids
     * @return
     */
    @AutoLog(value = "智慧教室ai服务器-批量重启")
    @ApiOperation(value = "智慧教室ai服务器-批量重启", notes = "智慧教室ai服务器-批量重启")
    @GetMapping(value = "/reconnectBatch")
    public Result<?> reconnectBatch(@RequestParam(name = "ids", required = true) String ids) {
        try {
            Collection<String> idsList = Arrays.asList(ids.split(","));
            int count = smcAiServerManagerService.reconnectBatch(idsList);
            if (count > 0) {
                return Result.OK(String.format("重启数量 %d/%d", count, idsList.size()));
            }
            return Result.error("重启失败!请检查服务器是否开启");
        } finally {
            smcAiServerManagerService.refreshAiServers();
        }
    }
}
