package org.jeecg.modules.smc.service.impl;

import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.core.conditions.update.UpdateWrapper;
import lombok.extern.slf4j.Slf4j;
import org.jeecg.common.util.DateUtils;
import org.jeecg.modules.smc.entity.SmcAiServer;
import org.jeecg.modules.smc.entity.SmcChtDet;
import org.jeecg.modules.smc.entity.SmcVideoSrc;
import org.jeecg.modules.smc.service.ISmcAiServerManagerService;
import org.jeecg.modules.smc.service.ISmcChtDetService;
import org.jeecg.modules.smc.service.ISmcChtDetTaskService;
import org.jeecg.modules.smc.service.ISmcVideoSrcService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.MediaType;
import org.springframework.stereotype.Service;
import org.springframework.util.LinkedMultiValueMap;
import org.springframework.util.MultiValueMap;
import org.springframework.web.client.RestTemplate;

import javax.annotation.PostConstruct;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

@Service
@Slf4j
public class SmcChtDetTaskServiceImpl implements ISmcChtDetTaskService {


    @Autowired
    private RestTemplate restTemplate;

    @Autowired
    private ISmcChtDetService smcChtDetService;

    @Autowired
    private ISmcVideoSrcService smcVideoSrcService;

    @Autowired
    private ISmcAiServerManagerService smcAiServerManagerService;

    private ScheduledExecutorService scheduledExecutorService;

    public final Map<String, String> taskRunning = new HashMap<>();

    @PostConstruct
    void initComponent() {
        scheduledExecutorService = Executors.newScheduledThreadPool(20);
        //resume
        QueryWrapper<SmcChtDet> queryWrapper = new QueryWrapper<>();
        queryWrapper.gt("end_time", DateUtils.now());
        List<SmcChtDet> resumeTasks = smcChtDetService.list(queryWrapper);
        saveBatch(resumeTasks);
    }

    @Override
    public void removeById(String taskId) {
        String url = null;
        synchronized (taskRunning) {
            if (taskRunning.containsKey(taskId)) {
                url = taskRunning.get(taskId);
                taskRunning.remove(taskId);
            }
        }
        smcChtDetService.removeById(taskId);
        if (url != null) {
            log.info("remove cheating detection " + taskId + ", " + url);
            try {
                restTemplate.delete(url);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    public void stopById(String taskId) {
        String url = null;
        synchronized (taskRunning) {
            if (taskRunning.containsKey(taskId)) {
                url = taskRunning.get(taskId);
                taskRunning.remove(taskId);
            }
        }
        if (url != null) {
            log.info("stop cheating detection " + taskId + ", " + url);
            try {
                restTemplate.delete(url);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    private void registerOneTask0(SmcChtDet smcChtDet, SmcAiServer aiServer) {
        //查询视频源信息
        SmcVideoSrc smcVideoSrc = smcVideoSrcService.getById(smcChtDet.getVideoSrcId());
        String videoSrcUrl = smcVideoSrc.getVideoUrl();
        int videoSrcType = smcVideoSrc.getVideoType();
        long endTime = smcChtDet.getEndTime().getTime();

        String serverUrl = aiServer.getServerUrl();


        String taskId = smcChtDet.getId();
        registerTaskRemoveHandler(taskId, serverUrl);
        long delayTime = smcChtDet.getStartTime().getTime() - System.currentTimeMillis();
        delayTime = Math.max(delayTime, 0);
        scheduledExecutorService.schedule(() -> {
            synchronized (taskRunning) {
                if (!taskRunning.containsKey(taskId)) {
                    return;
                }
//                taskRunning.remove(taskId); //启动后就不允许停止
            }
            try {
                MultiValueMap<String, Object> params = new LinkedMultiValueMap<>();
                params.add("src", videoSrcUrl);
                params.add("task_name", "chtdet_" + taskId);
                params.add("in_type", videoSrcType);
                params.add("end_time", endTime);


                HttpHeaders headers = new HttpHeaders();
                headers.setContentType(MediaType.APPLICATION_FORM_URLENCODED);
                restTemplate.postForEntity(
                        serverUrl + "/cht_det", new HttpEntity<>(params, headers),
                        String.class);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }, delayTime, TimeUnit.MILLISECONDS);
    }


    private void registerTaskRemoveHandler(String taskId, String serverUrl) {
        synchronized (taskRunning) {
            taskRunning.put(taskId, String.format("%s/stop_task?task_name=chtdet_%s", serverUrl, taskId));
        }
        log.info(String.format("注册任务 chtdet_%s 的删除句柄", taskId));
    }

    @Override
    public String save(SmcChtDet smcChtDet) {
        if (smcChtDet.getEndTime().before(new Date())) {
            return "无法添加过期的任务";  //任务已经结束
        }
        SmcAiServer aiServer = smcAiServerManagerService
                .isRunning(
                        smcChtDet.getAiServerId(),
                        "chtdet_" + smcChtDet.getId()
                );
        if (aiServer == null) {
            aiServer = smcAiServerManagerService.dispatchServer(smcChtDet);
            if (aiServer == null)
                return "AI资源分配失败";
            smcChtDet.setAiServerId(aiServer.getId());
            smcChtDetService.saveOrUpdate(smcChtDet);
            generateTaskNameIfEmpty(smcChtDet);
            registerOneTask0(smcChtDet, aiServer);
        } else {
            registerTaskRemoveHandler(smcChtDet.getId(), aiServer.getServerUrl());
        }
        return null;
    }

    private void generateTaskNameIfEmpty(SmcChtDet chtDet) {
        String taskName = chtDet.getTaskName();
        if (taskName == null || "".equals(taskName)) {
            String videoName = smcVideoSrcService.getById(chtDet.getVideoSrcId()).getVideoName();
            String newTaskName = String.format("作弊检测(%s)%s", videoName, DateUtils.now());
            UpdateWrapper<SmcChtDet> updateWrapper = new UpdateWrapper<>();
            updateWrapper.eq("id", chtDet.getId());
            updateWrapper.set("task_name", newTaskName);
            smcChtDetService.update(updateWrapper);
        }
    }
}
