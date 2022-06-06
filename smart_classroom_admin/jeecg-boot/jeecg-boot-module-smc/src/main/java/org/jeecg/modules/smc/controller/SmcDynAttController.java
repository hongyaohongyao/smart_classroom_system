package org.jeecg.modules.smc.controller;

import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.jeecg.common.api.vo.Result;
import org.jeecg.common.system.query.QueryGenerator;
import org.jeecg.common.util.oConvertUtils;
import org.jeecg.modules.smc.entity.SmcDynAtt;
import org.jeecg.modules.smc.service.ISmcDynAttService;

import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.core.metadata.IPage;
import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import lombok.extern.slf4j.Slf4j;

import org.jeecg.modules.smc.service.ISmcDynAttTaskService;
import org.jeecg.utils.ResultUtils;
import org.jeecgframework.poi.excel.ExcelImportUtil;
import org.jeecgframework.poi.excel.def.NormalExcelConstants;
import org.jeecgframework.poi.excel.entity.ExportParams;
import org.jeecgframework.poi.excel.entity.ImportParams;
import org.jeecgframework.poi.excel.view.JeecgEntityExcelView;
import org.jeecg.common.system.base.controller.JeecgController;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;
import org.springframework.web.multipart.MultipartFile;
import org.springframework.web.multipart.MultipartHttpServletRequest;
import org.springframework.web.servlet.ModelAndView;
import com.alibaba.fastjson.JSON;
import io.swagger.annotations.Api;
import io.swagger.annotations.ApiOperation;
import org.jeecg.common.aspect.annotation.AutoLog;

/**
 * @Description: 在线点名
 * @Author: jeecg-boot
 * @Date: 2022-03-14
 * @Version: V1.0
 */
@Api(tags = "在线点名")
@RestController
@RequestMapping("/smc/smcDynAtt")
@Slf4j
public class SmcDynAttController extends JeecgController<SmcDynAtt, ISmcDynAttService> {
    @Autowired
    private ISmcDynAttService smcDynAttService;

    @Autowired
    private ISmcDynAttTaskService smcDynAttTaskService;

    /**
     * 分页列表查询
     *
     * @param smcDynAtt
     * @param pageNo
     * @param pageSize
     * @param req
     * @return
     */
    @AutoLog(value = "在线点名-分页列表查询")
    @ApiOperation(value = "在线点名-分页列表查询", notes = "在线点名-分页列表查询")
    @GetMapping(value = "/list")
    public Result<?> queryPageList(SmcDynAtt smcDynAtt,
                                   @RequestParam(name = "pageNo", defaultValue = "1") Integer pageNo,
                                   @RequestParam(name = "pageSize", defaultValue = "10") Integer pageSize,
                                   HttpServletRequest req) {
        QueryWrapper<SmcDynAtt> queryWrapper = QueryGenerator.initQueryWrapper(smcDynAtt, req.getParameterMap());
        Page<SmcDynAtt> page = new Page<SmcDynAtt>(pageNo, pageSize);
        IPage<SmcDynAtt> pageList = smcDynAttService.page(page, queryWrapper);
        return Result.OK(pageList);
    }

    /**
     * 添加
     *
     * @param smcDynAtt
     * @return
     */
    @AutoLog(value = "在线点名-添加")
    @ApiOperation(value = "在线点名-添加", notes = "在线点名-添加")
    @PostMapping(value = "/add")
    public Result<?> add(@RequestBody SmcDynAtt smcDynAtt) {
        return ResultUtils.msgResult(smcDynAttTaskService.save(smcDynAtt), "添加成功！");
    }

    /**
     * 编辑
     *
     * @param smcDynAtt
     * @return
     */
    @AutoLog(value = "在线点名-编辑")
    @ApiOperation(value = "在线点名-编辑", notes = "在线点名-编辑")
    @PutMapping(value = "/edit")
    public Result<?> edit(@RequestBody SmcDynAtt smcDynAtt) {
        return Result.error("禁止编辑!");
    }

    /**
     * 通过id删除
     *
     * @param id
     * @return
     */
    @AutoLog(value = "在线点名-通过id删除")
    @ApiOperation(value = "在线点名-通过id删除", notes = "在线点名-通过id删除")
    @DeleteMapping(value = "/delete")
    public Result<?> delete(@RequestParam(name = "id", required = true) String id) {
        smcDynAttTaskService.removeById(id);
        return Result.OK("删除成功!");
    }

    /**
     * 批量删除
     *
     * @param ids
     * @return
     */
    @AutoLog(value = "在线点名-批量删除")
    @ApiOperation(value = "在线点名-批量删除", notes = "在线点名-批量删除")
    @DeleteMapping(value = "/deleteBatch")
    public Result<?> deleteBatch(@RequestParam(name = "ids", required = true) String ids) {
        smcDynAttTaskService.removeByIds(Arrays.asList(ids.split(",")));
        return Result.OK("批量删除成功!");
    }

    /**
     * 通过id查询
     *
     * @param id
     * @return
     */
    @AutoLog(value = "在线点名-通过id查询")
    @ApiOperation(value = "在线点名-通过id查询", notes = "在线点名-通过id查询")
    @GetMapping(value = "/queryById")
    public Result<?> queryById(@RequestParam(name = "id", required = true) String id) {
        SmcDynAtt smcDynAtt = smcDynAttService.getById(id);
        if (smcDynAtt == null) {
            return Result.error("未找到对应数据");
        }
        return Result.OK(smcDynAtt);
    }

    /**
     * 导出excel
     *
     * @param request
     * @param smcDynAtt
     */
    @RequestMapping(value = "/exportXls")
    public ModelAndView exportXls(HttpServletRequest request, SmcDynAtt smcDynAtt) {
        return super.exportXls(request, smcDynAtt, SmcDynAtt.class, "在线点名");
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
        return Result.error("禁止批量导入");
    }

}
