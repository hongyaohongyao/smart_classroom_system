package org.jeecg.modules.smc.service.impl;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.core.conditions.update.UpdateWrapper;
import com.baomidou.mybatisplus.core.toolkit.Assert;
import lombok.extern.slf4j.Slf4j;
import org.jeecg.common.util.DateUtils;
import org.jeecg.modules.smc.entity.*;
import org.jeecg.modules.smc.service.*;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Service;
import org.springframework.util.LinkedMultiValueMap;
import org.springframework.util.MultiValueMap;
import org.springframework.web.client.RestTemplate;

import javax.annotation.PostConstruct;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

@Service
@Slf4j
public class SmcDynAttTaskServiceImpl implements ISmcDynAttTaskService {

    @Autowired
    private RestTemplate restTemplate;

    @Autowired
    private ISmcDynAttService smcDynAttService;
    @Autowired
    private ISmcVideoSrcService smcVideoSrcService;
    @Autowired
    private ISmcAiServerManagerService smcAiServerManagerService;
    @Autowired
    private ISmcClassService smcClassService;
    @Autowired
    private ISmcFaceRegService smcFaceRegService;

    @Autowired
    private ISmcClassMemService smcClassMemService;


    public final Map<String, String> taskRunning = new HashMap<>();

    @PostConstruct
    void initComponent() {
        //resume
        QueryWrapper<SmcDynAtt> queryWrapper = new QueryWrapper<>();
        queryWrapper.gt("end_time", DateUtils.now());
        List<SmcDynAtt> resumeTasks = smcDynAttService.list(queryWrapper);
        saveBatch(resumeTasks);
    }

    @Override
    public boolean aliveById(String taskId) {
        String url = null;
        synchronized (taskRunning) {
            if (taskRunning.containsKey(taskId)) {
                url = taskRunning.get(taskId);
            }
        }
        if (url != null) {
            log.info("awaking dynamic attendance " + taskId + ", " + url);
            try {
                //更新任务结束时间为一分钟以后
                long newEndTime = System.currentTimeMillis() + 60000;
                ResponseEntity<String> resp = restTemplate.getForEntity(url, String.class, newEndTime);

                String result = resp.getBody();
                if ("ok".equals(result)) {
                    UpdateWrapper<SmcDynAtt> updateWrapper = new UpdateWrapper<>();
                    updateWrapper.eq("id", taskId);
                    updateWrapper.set("end_time",
                            DateUtils.datetimeFormat.get().format(new Date(newEndTime))
                    );
                    smcDynAttService.update(updateWrapper);
                    return true;
                } else {
                    synchronized (taskRunning) {
                        taskRunning.remove(taskId);
                    }
                    UpdateWrapper<SmcDynAtt> updateWrapper = new UpdateWrapper<>();
                    updateWrapper.eq("id", taskId);
                    updateWrapper.set("end_time", DateUtils.now());
                    smcDynAttService.update(updateWrapper);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return false;
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
            log.info("stopping dynamic attendance " + taskId + ", " + url);
            try {
                restTemplate.getForEntity(url, String.class, 0);
                UpdateWrapper<SmcDynAtt> updateWrapper = new UpdateWrapper<>();
                updateWrapper.eq("id", taskId);
                updateWrapper.set("end_time", DateUtils.now());
                smcDynAttService.update(updateWrapper);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
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
        smcDynAttService.removeById(taskId);
        if (url != null) {
            log.info("remove dynamic attendance " + taskId + ", " + url);
            try {
                restTemplate.getForEntity(url, String.class, 0);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    private void registerOneTask0(SmcDynAtt dynAtt, SmcAiServer aiServer) {
        //查询视频源信息
        SmcClass smcClass = smcClassService.getById(dynAtt.getClassId());
        SmcVideoSrc smcVideoSrc = smcVideoSrcService.getById(smcClass.getVideoSrcId());
        String videoSrcUrl = smcVideoSrc.getVideoUrl();
        int videoSrcType = smcVideoSrc.getVideoType();

        //查询人脸特征信息
        List<SmcClassMem> classMems = smcClassMemService.selectByMainId(dynAtt.getClassId());

        Assert.notEmpty(classMems, "未找到班级内成员");
        JSONArray memInfos = new JSONArray();
        JSONArray featureMap = new JSONArray();
        for (SmcClassMem classMem : classMems) {
            SmcFaceReg faceReg = smcFaceRegService.getById(classMem.getFaceRegId());
            if (faceReg == null) {
                continue;
            }
            String faceEncode = faceReg.getFaceEncode();
            JSONArray feature = JSON.parseArray(faceEncode);
            Assert.isTrue(feature != null && feature.size() == 512
                    , String.format("错误的人脸特征 %s", faceReg.getFaceName()));
            featureMap.add(feature);
            //添加名字和成员id
            JSONObject nameObj = new JSONObject();
            nameObj.put("name", faceReg.getFaceNickName());
            nameObj.put("memId", classMem.getId());
            memInfos.add(nameObj);
        }
        JSONObject attInfo = new JSONObject();
        attInfo.put("memInfos", memInfos);
        attInfo.put("featureMap", featureMap);

        long endTime = dynAtt.getEndTime().getTime();
        String serverUrl = aiServer.getServerUrl();
        String taskId = dynAtt.getId();

        MultiValueMap<String, String> params = new LinkedMultiValueMap<>();
        params.add("att_info", attInfo.toJSONString());
        params.add("src", videoSrcUrl);
        params.add("task_name", "dynatt_" + taskId);
        params.add("in_type", String.valueOf(videoSrcType));
        params.add("end_time", String.valueOf(endTime));

        HttpHeaders headers = new HttpHeaders();
        headers.setContentType(MediaType.APPLICATION_FORM_URLENCODED);

        ResponseEntity<String> resp =
                restTemplate.postForEntity(serverUrl + "/dyn_att",
                        new HttpEntity<>(params, headers), String.class);
//        ResponseEntity<String> resp = restTemplate.postForEntity(
//                serverUrl + "/dyn_att?src={?}" +
//                        "&task_name=dynatt_{?}" +
//                        "&in_type={?}" +
//                        "&end_time={?}",
//                params,
//                String.class,
//                videoSrcUrl,
//                taskId,
//                videoSrcType, endTime);
        String result = resp.getBody();
        Assert.isTrue("ok".equals(result), result == null ? "推理服务器无返回值" : result);
        registerTaskResetEndTimeHandler(taskId, serverUrl);
    }


    private void registerTaskResetEndTimeHandler(String taskId, String serverUrl) {
        synchronized (taskRunning) {
            taskRunning.put(taskId,
                    String.format("%s/reset_end_time?task_name=dynatt_%s&end_time={?}",
                            serverUrl, taskId
                    )
            );
        }
        log.info(String.format("注册任务 dynatt_%s 的时间刷新句柄", taskId));
    }

    @Override
    public String save(SmcDynAtt dynAtt) {
        if (dynAtt.getEndTime() == null) {
            dynAtt.setEndTime(new Date(System.currentTimeMillis() + 90000));
            smcDynAttService.saveOrUpdate(dynAtt);
        }
        if (new Date().after(dynAtt.getEndTime())) {
            return "无法添加过期的任务";  //任务已经结束
        }
        SmcAiServer aiServer = smcAiServerManagerService
                .isRunning(
                        dynAtt.getAiServerId(),
                        "dynatt_" + dynAtt.getId()
                );
        if (aiServer == null) {
            aiServer = smcAiServerManagerService.dispatchServer(dynAtt);
            if (aiServer == null)
                return "AI资源分配失败";
            dynAtt.setAiServerId(aiServer.getId());
            smcDynAttService.saveOrUpdate(dynAtt);
            generateTaskNameIfEmpty(dynAtt);
            registerOneTask0(dynAtt, aiServer);
        } else {
            registerTaskResetEndTimeHandler(dynAtt.getId(), aiServer.getServerUrl());
        }
        return null;
    }

    private void generateTaskNameIfEmpty(SmcDynAtt dynAtt) {
        String taskName = dynAtt.getTaskName();
        if (taskName == null || "".equals(taskName)) {
            String className = smcClassService.getById(dynAtt.getClassId()).getClassName();
            String newTaskName = String.format("动态点名(%s)%s", className, DateUtils.now());
            UpdateWrapper<SmcDynAtt> updateWrapper = new UpdateWrapper<>();
            updateWrapper.eq("id", dynAtt.getId());
            updateWrapper.set("task_name", newTaskName);
            smcDynAttService.update(updateWrapper);
        }
    }
}
