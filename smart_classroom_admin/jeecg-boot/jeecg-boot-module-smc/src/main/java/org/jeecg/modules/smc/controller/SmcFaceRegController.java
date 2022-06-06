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
import org.jeecg.modules.smc.entity.SmcFaceReg;
import org.jeecg.modules.smc.service.ISmcFaceEncoderService;
import org.jeecg.modules.smc.service.ISmcFaceRegService;

import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.core.metadata.IPage;
import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import lombok.extern.slf4j.Slf4j;

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
 * @Description: 人脸注册
 * @Author: jeecg-boot
 * @Date: 2022-03-11
 * @Version: V1.0
 */
@Api(tags = "人脸注册")
@RestController
@RequestMapping("/smc/smcFaceReg")
@Slf4j
public class SmcFaceRegController extends JeecgController<SmcFaceReg, ISmcFaceRegService> {
    @Autowired
    private ISmcFaceRegService smcFaceRegService;

    @Autowired
    private ISmcFaceEncoderService smcFaceEncoderService;

    /**
     * 分页列表查询
     *
     * @param smcFaceReg
     * @param pageNo
     * @param pageSize
     * @param req
     * @return
     */
    @AutoLog(value = "人脸注册-分页列表查询")
    @ApiOperation(value = "人脸注册-分页列表查询", notes = "人脸注册-分页列表查询")
    @GetMapping(value = "/list")
    public Result<?> queryPageList(SmcFaceReg smcFaceReg,
                                   @RequestParam(name = "pageNo", defaultValue = "1") Integer pageNo,
                                   @RequestParam(name = "pageSize", defaultValue = "10") Integer pageSize,
                                   HttpServletRequest req) {
        QueryWrapper<SmcFaceReg> queryWrapper = QueryGenerator.initQueryWrapper(smcFaceReg, req.getParameterMap());
        Page<SmcFaceReg> page = new Page<SmcFaceReg>(pageNo, pageSize);
        IPage<SmcFaceReg> pageList = smcFaceRegService.page(page, queryWrapper);
        return Result.OK(pageList);
    }

    /**
     * 添加
     *
     * @param smcFaceReg
     * @return
     */
    @AutoLog(value = "人脸注册-添加")
    @ApiOperation(value = "人脸注册-添加", notes = "人脸注册-添加")
    @PostMapping(value = "/add")
    public Result<?> add(@RequestBody SmcFaceReg smcFaceReg) {
        String result = smcFaceEncoderService.encodeAndSaveFace(smcFaceReg);
        return result == null ? Result.OK("添加成功！") : Result.error(result);
    }

    /**
     * 编辑
     *
     * @param smcFaceReg
     * @return
     */
    @AutoLog(value = "人脸注册-编辑")
    @ApiOperation(value = "人脸注册-编辑", notes = "人脸注册-编辑")
    @PutMapping(value = "/edit")
    public Result<?> edit(@RequestBody SmcFaceReg smcFaceReg) {
        String result = smcFaceEncoderService.encodeAndSaveFace(smcFaceReg);
        return result == null ? Result.OK("编辑成功!") : Result.error(result);
    }

    /**
     * 通过id删除
     *
     * @param id
     * @return
     */
    @AutoLog(value = "人脸注册-通过id删除")
    @ApiOperation(value = "人脸注册-通过id删除", notes = "人脸注册-通过id删除")
    @DeleteMapping(value = "/delete")
    public Result<?> delete(@RequestParam(name = "id", required = true) String id) {
        smcFaceRegService.removeById(id);
        return Result.OK("删除成功!");
    }

    /**
     * 批量删除
     *
     * @param ids
     * @return
     */
    @AutoLog(value = "人脸注册-批量删除")
    @ApiOperation(value = "人脸注册-批量删除", notes = "人脸注册-批量删除")
    @DeleteMapping(value = "/deleteBatch")
    public Result<?> deleteBatch(@RequestParam(name = "ids", required = true) String ids) {
        this.smcFaceRegService.removeByIds(Arrays.asList(ids.split(",")));
        return Result.OK("批量删除成功!");
    }

    /**
     * 通过id查询
     *
     * @param id
     * @return
     */
    @AutoLog(value = "人脸注册-通过id查询")
    @ApiOperation(value = "人脸注册-通过id查询", notes = "人脸注册-通过id查询")
    @GetMapping(value = "/queryById")
    public Result<?> queryById(@RequestParam(name = "id", required = true) String id) {
        SmcFaceReg smcFaceReg = smcFaceRegService.getById(id);
        if (smcFaceReg == null) {
            return Result.error("未找到对应数据");
        }
        return Result.OK(smcFaceReg);
    }

    /**
     * 导出excel
     *
     * @param request
     * @param smcFaceReg
     */
    @RequestMapping(value = "/exportXls")
    public ModelAndView exportXls(HttpServletRequest request, SmcFaceReg smcFaceReg) {
        return super.exportXls(request, smcFaceReg, SmcFaceReg.class, "人脸注册");
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
        return super.importExcel(request, response, SmcFaceReg.class);
    }

}
