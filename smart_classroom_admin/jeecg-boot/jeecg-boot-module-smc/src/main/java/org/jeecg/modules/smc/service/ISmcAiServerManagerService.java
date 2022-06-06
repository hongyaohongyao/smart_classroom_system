package org.jeecg.modules.smc.service;

import org.jeecg.modules.smc.entity.SmcAiServer;
import org.springframework.transaction.annotation.Transactional;

import java.util.Collection;

/**
 * ai服务器的分配和连接管理
 *
 * @author hongyaohongyao
 */
public interface ISmcAiServerManagerService {

    /**
     * 随机分配一个服务器
     *
     * @param obj
     * @return
     */
    @Transactional(rollbackFor = {Exception.class})
    SmcAiServer dispatchServer(Object obj);

    /**
     * 判断服务器上是否在跑指定任务名，并获取服务器对象
     *
     * @param aiServerId
     * @param taskNameInServer
     * @return 正在跑任务时返回服务器对象，其余情况返回null
     */
    SmcAiServer isRunning(String aiServerId, String taskNameInServer);

    /**
     * 通知Manager重新获取使用Ai服务器(在内部获取所有状态为0的服务器对象)
     */
    void refreshAiServers();

    /**
     * 尝试重连指定id的服务器
     *
     * @param aiServerId
     * @return
     */
    @Transactional(rollbackFor = {Exception.class})
    boolean reconnect(String aiServerId);

    default int reconnectBatch(Collection<String> aiServerIds) {
        int count = 0;
        for (String aiServerId : aiServerIds) {
            try {
                if (reconnect(aiServerId)) {
                    count++;
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return count;
    }
}
