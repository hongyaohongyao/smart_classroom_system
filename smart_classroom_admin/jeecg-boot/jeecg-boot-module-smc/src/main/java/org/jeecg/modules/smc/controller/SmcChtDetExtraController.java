package org.jeecg.modules.smc.controller;

import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.core.metadata.IPage;
import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import io.swagger.annotations.Api;
import io.swagger.annotations.ApiOperation;
import lombok.extern.slf4j.Slf4j;
import org.jeecg.common.api.vo.Result;
import org.jeecg.common.aspect.annotation.AutoLog;
import org.jeecg.common.system.base.controller.JeecgController;
import org.jeecg.common.system.query.QueryGenerator;
import org.jeecg.common.util.DateUtils;
import org.jeecg.modules.smc.entity.SmcChtDet;
import org.jeecg.modules.smc.service.ISmcChtDetService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import javax.servlet.http.HttpServletRequest;

/**
 * @Description: 作弊检测任务额外功能
 * @Author: hongyaohongyao
 * @Date: 2022-02-25
 * @Version: V1.0
 */
@Api(tags = "作弊检测任务附加功能")
@RestController
@RequestMapping("/smc/smcChtDetExtra")
@Slf4j
public class SmcChtDetExtraController extends JeecgController<SmcChtDet, ISmcChtDetService> {
    @Autowired
    private ISmcChtDetService smcChtDetService;

    /**
     * 分页列表查询
     *
     * @param smcChtDet
     * @param pageNo
     * @param pageSize
     * @param req
     * @return
     */
    @AutoLog(value = "正在运行的作弊检测任务-分页列表查询")
    @ApiOperation(value = "正在运行的作弊检测任务-分页列表查询", notes = "正在运行的作弊检测任务-分页列表查询")
    @GetMapping(value = "/list_running")
    public Result<?> queryPageListRunning(SmcChtDet smcChtDet,
                                          @RequestParam(name = "pageNo", defaultValue = "1") Integer pageNo,
                                          @RequestParam(name = "pageSize", defaultValue = "10") Integer pageSize,
                                          HttpServletRequest req) {
        QueryWrapper<SmcChtDet> queryWrapper = QueryGenerator.initQueryWrapper(smcChtDet, req.getParameterMap());
        String now = DateUtils.now();
        queryWrapper.le("start_time", now).gt("end_time", now);
        Page<SmcChtDet> page = new Page<SmcChtDet>(pageNo, pageSize);
        IPage<SmcChtDet> pageList = smcChtDetService.page(page, queryWrapper);
        return Result.OK(pageList);
    }


}
