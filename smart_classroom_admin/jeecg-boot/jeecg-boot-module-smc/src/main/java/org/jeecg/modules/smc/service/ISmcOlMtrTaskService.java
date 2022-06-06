package org.jeecg.modules.smc.service;

import org.jeecg.common.api.vo.Result;
import org.jeecg.modules.smc.entity.SmcOlMtr;
import org.jeecgframework.poi.excel.ExcelImportUtil;
import org.jeecgframework.poi.excel.entity.ImportParams;
import org.springframework.transaction.annotation.Transactional;
import org.springframework.web.multipart.MultipartFile;
import org.springframework.web.multipart.MultipartHttpServletRequest;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;
import java.util.Collection;
import java.util.List;
import java.util.Map;

public interface ISmcOlMtrTaskService {

    @Transactional(
            rollbackFor = {Exception.class}
    )
    String nextState(String taskId);

    @Transactional(
            rollbackFor = {Exception.class}
    )
    void removeById(String taskId);

    default void removeByIds(Collection<String> taskIds) {
        for (String taskId : taskIds) {
            try {
                removeById(taskId);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    @Transactional(
            rollbackFor = {Exception.class}
    )
    String save(SmcOlMtr smcOlMtr);

    default void saveBatch(Collection<SmcOlMtr> smcOlMtrs) {
        for (SmcOlMtr smcOlMtr : smcOlMtrs) {
            try {
                save(smcOlMtr);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    default Result<?> importExcel(HttpServletRequest request, HttpServletResponse response, Class<SmcOlMtr> clazz) {
        MultipartHttpServletRequest multipartRequest = (MultipartHttpServletRequest) request;
        Map<String, MultipartFile> fileMap = multipartRequest.getFileMap();
        for (Map.Entry<String, MultipartFile> entity : fileMap.entrySet()) {
            MultipartFile file = entity.getValue();// 获取上传文件对象
            ImportParams params = new ImportParams();
            params.setTitleRows(2);
            params.setHeadRows(1);
            params.setNeedSave(true);
            try {
                List<SmcOlMtr> list = ExcelImportUtil.importExcel(file.getInputStream(), clazz, params);
                saveBatch(list);
                return Result.ok("文件导入成功！数据行数：" + list.size());
            } catch (Exception e) {
                return Result.error("文件导入失败:" + e.getMessage());
            } finally {
                try {
                    file.getInputStream().close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return Result.error("文件导入失败！");
    }

}
