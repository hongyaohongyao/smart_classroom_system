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
import org.jeecg.modules.smc.entity.SmcOlMtr;
import org.jeecg.modules.smc.service.ISmcOlMtrService;
import org.jeecg.modules.smc.service.ISmcOlMtrTaskService;
import org.jeecg.utils.ResultUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;
import org.springframework.web.servlet.ModelAndView;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.Arrays;

/**
 * @Description: 智慧教室在线监考
 * @Author: jeecg-boot
 * @Date: 2022-04-16
 * @Version: V1.0
 */
@Api(tags = "智慧教室在线监考")
@RestController
@RequestMapping("/smc/smcOlMtr")
@Slf4j
public class SmcOlMtrController extends JeecgController<SmcOlMtr, ISmcOlMtrService> {
    @Autowired
    private ISmcOlMtrService smcOlMtrService;

    @Autowired
    private ISmcOlMtrTaskService smcOlMtrTaskService;

    /**
     * 分页列表查询
     *
     * @param smcOlMtr
     * @param pageNo
     * @param pageSize
     * @param req
     * @return
     */
    @AutoLog(value = "智慧教室在线监考-分页列表查询")
    @ApiOperation(value = "智慧教室在线监考-分页列表查询", notes = "智慧教室在线监考-分页列表查询")
    @GetMapping(value = "/list")
    public Result<?> queryPageList(SmcOlMtr smcOlMtr,
                                   @RequestParam(name = "pageNo", defaultValue = "1") Integer pageNo,
                                   @RequestParam(name = "pageSize", defaultValue = "10") Integer pageSize,
                                   HttpServletRequest req) {
        QueryWrapper<SmcOlMtr> queryWrapper = QueryGenerator.initQueryWrapper(smcOlMtr, req.getParameterMap());
        Page<SmcOlMtr> page = new Page<SmcOlMtr>(pageNo, pageSize);
        IPage<SmcOlMtr> pageList = smcOlMtrService.page(page, queryWrapper);
        return Result.OK(pageList);
    }

    /**
     * 添加
     *
     * @param smcOlMtr
     * @return
     */
    @AutoLog(value = "智慧教室在线监考-添加")
    @ApiOperation(value = "智慧教室在线监考-添加", notes = "智慧教室在线监考-添加")
    @PostMapping(value = "/add")
    public Result<?> add(@RequestBody SmcOlMtr smcOlMtr) {
        return ResultUtils.msgResult(smcOlMtrTaskService.save(smcOlMtr), "添加成功");
    }

    /**
     * 编辑
     *
     * @param smcOlMtr
     * @return
     */
    @AutoLog(value = "智慧教室在线监考-编辑")
    @ApiOperation(value = "智慧教室在线监考-编辑", notes = "智慧教室在线监考-编辑")
    @PutMapping(value = "/edit")
    public Result<?> edit(@RequestBody SmcOlMtr smcOlMtr) {
        return Result.error("禁止编辑!");
    }

    /**
     * 通过id删除
     *
     * @param id
     * @return
     */
    @AutoLog(value = "智慧教室在线监考-通过id删除")
    @ApiOperation(value = "智慧教室在线监考-通过id删除", notes = "智慧教室在线监考-通过id删除")
    @DeleteMapping(value = "/delete")
    public Result<?> delete(@RequestParam(name = "id", required = true) String id) {
        smcOlMtrTaskService.removeById(id);
        return Result.OK("删除成功!");
    }

    /**
     * 批量删除
     *
     * @param ids
     * @return
     */
    @AutoLog(value = "智慧教室在线监考-批量删除")
    @ApiOperation(value = "智慧教室在线监考-批量删除", notes = "智慧教室在线监考-批量删除")
    @DeleteMapping(value = "/deleteBatch")
    public Result<?> deleteBatch(@RequestParam(name = "ids", required = true) String ids) {
        this.smcOlMtrTaskService.removeByIds(Arrays.asList(ids.split(",")));
        return Result.OK("批量删除成功!");
    }

    /**
     * 通过id查询
     *
     * @param id
     * @return
     */
    @AutoLog(value = "智慧教室在线监考-通过id查询")
    @ApiOperation(value = "智慧教室在线监考-通过id查询", notes = "智慧教室在线监考-通过id查询")
    @GetMapping(value = "/queryById")
    public Result<?> queryById(@RequestParam(name = "id", required = true) String id) {
        SmcOlMtr smcOlMtr = smcOlMtrService.getById(id);
        if (smcOlMtr == null) {
            return Result.error("未找到对应数据");
        }
        return Result.OK(smcOlMtr);
    }

    /**
     * 导出excel
     *
     * @param request
     * @param smcOlMtr
     */
    @RequestMapping(value = "/exportXls")
    public ModelAndView exportXls(HttpServletRequest request, SmcOlMtr smcOlMtr) {
        return super.exportXls(request, smcOlMtr, SmcOlMtr.class, "智慧教室在线监考");
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
        return Result.error("暂不支持批量导入");
//        return super.importExcel(request, response, SmcOlMtr.class);
    }

}
