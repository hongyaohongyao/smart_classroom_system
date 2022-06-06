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
import org.jeecg.modules.smc.entity.SmcChtDet;
import org.jeecg.modules.smc.service.ISmcChtDetService;
import org.jeecg.modules.smc.service.ISmcChtDetTaskService;
import org.jeecg.utils.ResultUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;
import org.springframework.web.servlet.ModelAndView;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.Arrays;

/**
 * @Description: 作弊检测任务
 * @Author: jeecg-boot
 * @Date: 2022-02-25
 * @Version: V1.0
 */
@Api(tags = "作弊检测任务")
@RestController
@RequestMapping("/smc/smcChtDet")
@Slf4j
public class SmcChtDetController extends JeecgController<SmcChtDet, ISmcChtDetService> {
    @Autowired
    private ISmcChtDetService smcChtDetService;

    @Autowired
    private ISmcChtDetTaskService smcChtDetTaskService;

    /**
     * 分页列表查询
     *
     * @param smcChtDet
     * @param pageNo
     * @param pageSize
     * @param req
     * @return
     */
    @AutoLog(value = "作弊检测任务-分页列表查询")
    @ApiOperation(value = "作弊检测任务-分页列表查询", notes = "作弊检测任务-分页列表查询")
    @GetMapping(value = "/list")
    public Result<?> queryPageList(SmcChtDet smcChtDet,
                                   @RequestParam(name = "pageNo", defaultValue = "1") Integer pageNo,
                                   @RequestParam(name = "pageSize", defaultValue = "10") Integer pageSize,
                                   HttpServletRequest req) {
        QueryWrapper<SmcChtDet> queryWrapper = QueryGenerator.initQueryWrapper(smcChtDet, req.getParameterMap());
        Page<SmcChtDet> page = new Page<SmcChtDet>(pageNo, pageSize);
        IPage<SmcChtDet> pageList = smcChtDetService.page(page, queryWrapper);
        return Result.OK(pageList);
    }

    /**
     * 添加
     *
     * @param smcChtDet
     * @return
     */
    @AutoLog(value = "作弊检测任务-添加")
    @ApiOperation(value = "作弊检测任务-添加", notes = "作弊检测任务-添加")
    @PostMapping(value = "/add")
    public Result<?> add(@RequestBody SmcChtDet smcChtDet) {
        return ResultUtils.msgResult(smcChtDetTaskService.save(smcChtDet), "添加成功!");
    }

    /**
     * 编辑
     *
     * @param smcChtDet
     * @return
     */
    @AutoLog(value = "作弊检测任务-编辑")
    @ApiOperation(value = "作弊检测任务-编辑", notes = "作弊检测任务-编辑")
    @PutMapping(value = "/edit")
    public Result<?> edit(@RequestBody SmcChtDet smcChtDet) {
        return Result.error("禁止编辑");
    }

    /**
     * 通过id删除
     *
     * @param id
     * @return
     */
    @AutoLog(value = "作弊检测任务-通过id删除")
    @ApiOperation(value = "作弊检测任务-通过id删除", notes = "作弊检测任务-通过id删除")
    @DeleteMapping(value = "/delete")
    public Result<?> delete(@RequestParam(name = "id", required = true) String id) {
        smcChtDetTaskService.removeById(id);
        return Result.OK("删除成功!");
    }

    /**
     * 批量删除
     *
     * @param ids
     * @return
     */
    @AutoLog(value = "作弊检测任务-批量删除")
    @ApiOperation(value = "作弊检测任务-批量删除", notes = "作弊检测任务-批量删除")
    @DeleteMapping(value = "/deleteBatch")
    public Result<?> deleteBatch(@RequestParam(name = "ids", required = true) String ids) {
        this.smcChtDetTaskService.removeByIds(Arrays.asList(ids.split(",")));
        return Result.OK("批量删除成功!");
    }

    /**
     * 通过id查询
     *
     * @param id
     * @return
     */
    @AutoLog(value = "作弊检测任务-通过id查询")
    @ApiOperation(value = "作弊检测任务-通过id查询", notes = "作弊检测任务-通过id查询")
    @GetMapping(value = "/queryById")
    public Result<?> queryById(@RequestParam(name = "id", required = true) String id) {
        SmcChtDet smcChtDet = smcChtDetService.getById(id);
        if (smcChtDet == null) {
            return Result.error("未找到对应数据");
        }
        return Result.OK(smcChtDet);
    }

    /**
     * 导出excel
     *
     * @param request
     * @param smcChtDet
     */
    @RequestMapping(value = "/exportXls")
    public ModelAndView exportXls(HttpServletRequest request, SmcChtDet smcChtDet) {
        return super.exportXls(request, smcChtDet, SmcChtDet.class, "作弊检测任务");
    }

    /**
     * 通过excel导入数据
     *
     * @param request
     * @param response
     * @return
     */
    @RequestMapping(value = "/importExcel", method = RequestMethod.POST)
    public Result<?> importExcel(HttpServletRequest request, HttpServletResponse response) {
        return smcChtDetTaskService.importExcel(request, response, SmcChtDet.class);
    }

}
