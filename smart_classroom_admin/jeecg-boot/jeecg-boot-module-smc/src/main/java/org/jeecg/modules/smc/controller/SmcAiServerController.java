package org.jeecg.modules.smc.controller;

import java.util.Arrays;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.jeecg.common.api.vo.Result;
import org.jeecg.common.system.query.QueryGenerator;

import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.core.metadata.IPage;
import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import lombok.extern.slf4j.Slf4j;

import org.jeecg.common.system.base.controller.JeecgController;
import org.jeecg.modules.smc.entity.SmcAiServer;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;
import org.springframework.web.servlet.ModelAndView;
import io.swagger.annotations.Api;
import io.swagger.annotations.ApiOperation;
import org.jeecg.common.aspect.annotation.AutoLog;
import org.jeecg.modules.smc.service.ISmcAiServerManagerService;
import org.jeecg.modules.smc.service.ISmcAiServerService;

/**
 * @Description: 智慧教室ai服务器
 * @Author: jeecg-boot
 * @Date: 2022-02-25
 * @Version: V1.0
 */
@Api(tags = "智慧教室ai服务器")
@RestController
@RequestMapping("/smc/smcAiServer")
@Slf4j
public class SmcAiServerController extends JeecgController<SmcAiServer, ISmcAiServerService> {
    @Autowired
    private ISmcAiServerService smcAiServerService;

    @Autowired
    private ISmcAiServerManagerService smcAiServerManagerService;

    /**
     * 分页列表查询
     *
     * @param smcAiServer
     * @param pageNo
     * @param pageSize
     * @param req
     * @return
     */
    @AutoLog(value = "智慧教室ai服务器-分页列表查询")
    @ApiOperation(value = "智慧教室ai服务器-分页列表查询", notes = "智慧教室ai服务器-分页列表查询")
    @GetMapping(value = "/list")
    public Result<?> queryPageList(SmcAiServer smcAiServer,
                                   @RequestParam(name = "pageNo", defaultValue = "1") Integer pageNo,
                                   @RequestParam(name = "pageSize", defaultValue = "10") Integer pageSize,
                                   HttpServletRequest req) {
        QueryWrapper<SmcAiServer> queryWrapper = QueryGenerator.initQueryWrapper(smcAiServer, req.getParameterMap());
        Page<SmcAiServer> page = new Page<SmcAiServer>(pageNo, pageSize);
        IPage<SmcAiServer> pageList = smcAiServerService.page(page, queryWrapper);
        return Result.OK(pageList);
    }

    /**
     * 添加
     *
     * @param smcAiServer
     * @return
     */
    @AutoLog(value = "智慧教室ai服务器-添加")
    @ApiOperation(value = "智慧教室ai服务器-添加", notes = "智慧教室ai服务器-添加")
    @PostMapping(value = "/add")
    public Result<?> add(@RequestBody SmcAiServer smcAiServer) {
        smcAiServerService.save(smcAiServer);
        smcAiServerManagerService.refreshAiServers();
        return Result.OK("添加成功！");
    }

    /**
     * 编辑
     *
     * @param smcAiServer
     * @return
     */
    @AutoLog(value = "智慧教室ai服务器-编辑")
    @ApiOperation(value = "智慧教室ai服务器-编辑", notes = "智慧教室ai服务器-编辑")
    @PutMapping(value = "/edit")
    public Result<?> edit(@RequestBody SmcAiServer smcAiServer) {
        smcAiServerService.updateById(smcAiServer);
        smcAiServerManagerService.refreshAiServers();
        return Result.OK("编辑成功!");
    }

    /**
     * 通过id删除
     *
     * @param id
     * @return
     */
    @AutoLog(value = "智慧教室ai服务器-通过id删除")
    @ApiOperation(value = "智慧教室ai服务器-通过id删除", notes = "智慧教室ai服务器-通过id删除")
    @DeleteMapping(value = "/delete")
    public Result<?> delete(@RequestParam(name = "id", required = true) String id) {
        smcAiServerService.removeById(id);
        smcAiServerManagerService.refreshAiServers();
        return Result.OK("删除成功!");
    }

    /**
     * 批量删除
     *
     * @param ids
     * @return
     */
    @AutoLog(value = "智慧教室ai服务器-批量删除")
    @ApiOperation(value = "智慧教室ai服务器-批量删除", notes = "智慧教室ai服务器-批量删除")
    @DeleteMapping(value = "/deleteBatch")
    public Result<?> deleteBatch(@RequestParam(name = "ids", required = true) String ids) {
        this.smcAiServerService.removeByIds(Arrays.asList(ids.split(",")));
        smcAiServerManagerService.refreshAiServers();
        return Result.OK("批量删除成功!");
    }

    /**
     * 通过id查询
     *
     * @param id
     * @return
     */
    @AutoLog(value = "智慧教室ai服务器-通过id查询")
    @ApiOperation(value = "智慧教室ai服务器-通过id查询", notes = "智慧教室ai服务器-通过id查询")
    @GetMapping(value = "/queryById")
    public Result<?> queryById(@RequestParam(name = "id", required = true) String id) {
        SmcAiServer smcAiServer = smcAiServerService.getById(id);
        if (smcAiServer == null) {
            return Result.error("未找到对应数据");
        }
        return Result.OK(smcAiServer);
    }

    /**
     * 导出excel
     *
     * @param request
     * @param smcAiServer
     */
    @RequestMapping(value = "/exportXls")
    public ModelAndView exportXls(HttpServletRequest request, SmcAiServer smcAiServer) {
        return super.exportXls(request, smcAiServer, SmcAiServer.class, "智慧教室ai服务器");
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
        Result<?> result = super.importExcel(request, response, SmcAiServer.class);
        smcAiServerManagerService.refreshAiServers();
        return result;
    }

}
