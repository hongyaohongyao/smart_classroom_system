package org.jeecg.modules.smc.service.impl;

import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.core.conditions.update.UpdateWrapper;
import lombok.extern.slf4j.Slf4j;
import org.jeecg.modules.smc.entity.SmcAiServer;
import org.jeecg.modules.smc.service.ISmcChtDetService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Service;
import org.springframework.web.client.RestTemplate;
import org.jeecg.modules.smc.service.ISmcAiServerManagerService;
import org.jeecg.modules.smc.service.ISmcAiServerService;

import javax.annotation.PostConstruct;
import java.util.ArrayList;
import java.util.List;

@Service
@Slf4j
public class SmcAiServerManagerServiceImpl implements ISmcAiServerManagerService {
    @Autowired
    private ISmcAiServerService smcAiServerService;
    @Autowired
    private ISmcChtDetService smcChtDetService;

    @Autowired
    private RestTemplate restTemplate;

    private final List<SmcAiServer> aiServers = new ArrayList<>();

    @PostConstruct
    private void initComponent() {
        refreshAiServers();
    }

    @Override
    public SmcAiServer dispatchServer(Object obj) {
        if (aiServers.size() == 0) {
            return null;
        }
        SmcAiServer selected = null;
        for (; ; ) {
            synchronized (aiServers) {
                if (aiServers.size() <= 0) {
                    break;
                }
                int idx = (obj.hashCode() % aiServers.size() + aiServers.size()) % aiServers.size();
                log.info("访问ai服务器索引 {}", idx);
                selected = aiServers.get(idx);
            }
            try {
                if (testAiServer(selected)) {
                    break;
                }
            } catch (Exception e) {
                e.printStackTrace();
                setAiServerBad(selected);
            }
            aiServers.remove(selected);
            selected = null;
            log.info("ai服务器分配失败.剩余资源数{}", aiServers.size());
        }
        if (selected == null)
            return null;
        return selected;
    }


    /**
     * 任意连接失败后需要将服务器的状态置为1
     *
     * @param aiServer
     */
    private void setAiServerBad(SmcAiServer aiServer) {
        if (aiServer == null)
            return;
        UpdateWrapper<SmcAiServer> updateWrapper = new UpdateWrapper<>();
        updateWrapper.eq("id", aiServer.getId()).set("server_state", 1);
        smcAiServerService.update(updateWrapper);
        log.info("AI服务器 {}({}) 无法访问", aiServer.getServerName(), aiServer.getServerUrl());
    }


    private boolean testAiServer(SmcAiServer aiServer) {
        if (aiServer == null)
            return false;
        String url = String.format("%s/is_alive", aiServer.getServerUrl());
        ResponseEntity<String> response = restTemplate.getForEntity(url, String.class);
        String result = response.getBody();
        return "ok".equals(result);
    }

    /**
     * 尝试重新连接服务器，如果成功则将服务器置为0
     *
     * @param aiServer
     * @return
     */
    private boolean setAiServerGood(SmcAiServer aiServer) {
        try {
            if (testAiServer(aiServer)) {
                UpdateWrapper<SmcAiServer> updateWrapper = new UpdateWrapper<>();
                updateWrapper.eq("id", aiServer.getId()).set("server_state", 0);
                smcAiServerService.update(updateWrapper);
                return true;
            }
        } catch (Exception e) {
            e.printStackTrace();
            setAiServerBad(aiServer);
        }
        return false;
    }

    @Override
    public boolean reconnect(String aiServerId) {
        SmcAiServer aiServer = smcAiServerService.getById(aiServerId);
        return setAiServerGood(aiServer);
    }


    @Override
    public SmcAiServer isRunning(String aiServerId, String taskNameInServer) {
        if (aiServerId == null || "".equals(aiServerId)) {
            return null;
        }
        SmcAiServer aiServer = smcAiServerService.getById(aiServerId);
        if (aiServer == null) {
            return null;
        }
        try {
            if (hasTask(aiServer, taskNameInServer)) {
                return aiServer;
            }
        } catch (Exception e) {
            e.printStackTrace();
            setAiServerBad(aiServer);
        }
        return null;
    }

    private boolean hasTask(SmcAiServer aiServer, String taskNameInServer) {
        String url = String.format("%s/has_task?task_name={?}", aiServer.getServerUrl());
        ResponseEntity<String> response = restTemplate.getForEntity(url, String.class, taskNameInServer);
        String result = response.getBody();
        return "ok".equals(result);
    }


    @Override
    public void refreshAiServers() {
        synchronized (aiServers) {
            aiServers.clear();
            QueryWrapper<SmcAiServer> queryWrapper = new QueryWrapper<>();
            queryWrapper.eq("server_state", 0);
            aiServers.addAll(smcAiServerService.list(queryWrapper));
        }
        log.info("AI服务器资源已更新, 注册数量 {}", aiServers.size());
    }
}
