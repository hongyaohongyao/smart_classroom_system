package org.jeecg.modules.smc.service.impl;

import com.baomidou.mybatisplus.core.conditions.update.UpdateWrapper;
import org.jeecg.common.util.DateUtils;
import org.jeecg.modules.smc.entity.SmcChtDet;
import org.jeecg.modules.smc.entity.SmcClass;
import org.jeecg.modules.smc.entity.SmcDynAtt;
import org.jeecg.modules.smc.entity.SmcOlMtr;
import org.jeecg.modules.smc.service.*;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.Date;

@Service
public class SmcOlMtrTaskServiceImpl implements ISmcOlMtrTaskService {

    public static int OL_MTR_STATUS_NOT_START = 0;
    public static int OL_MTR_STATUS_DYN_ATT = 1;
    public static int OL_MTR_STATUS_CHT_DET = 2;
    public static int OL_MTR_STATUS_FINISH = 3;


    @Autowired
    ISmcVideoSrcService smcVideoSrcService;
    @Autowired
    ISmcOlMtrService smcOlMtrService;
    @Autowired
    ISmcClassService smcClassService;
    /*作弊检测相关服务*/
    @Autowired
    ISmcChtDetTaskService smcChtDetTaskService;
    @Autowired
    ISmcChtDetService smcChtDetService;
    /*动态点名相关服务*/
    @Autowired
    ISmcDynAttTaskService smcDynAttTaskService;
    @Autowired
    ISmcDynAttService smcDynAttService;

    @Override
    public String nextState(String taskId) {
        SmcOlMtr olMtr = smcOlMtrService.getById(taskId);
        if (olMtr == null)
            return "任务不存在";

        int status = olMtr.getTaskStatus();
        if (status == OL_MTR_STATUS_FINISH) {
            return "任务已结束";
        }
        String result = null;
        if (status == OL_MTR_STATUS_NOT_START) {
            result = caseToDynAtt(olMtr);
        } else if (status == OL_MTR_STATUS_DYN_ATT) {
            result = caseToChtDet(olMtr);
        } else if (status == OL_MTR_STATUS_CHT_DET) {
            result = caseToFinish(olMtr);
        } else {
            return "未处理的状态";
        }
        status++;

        if (result != null)
            return result;

        //更新状态
        if (status != olMtr.getTaskStatus()) {
            UpdateWrapper<SmcOlMtr> updateWrapper = new UpdateWrapper<>();
            updateWrapper.set("task_status", status);
            updateWrapper.eq("id", taskId);
            smcOlMtrService.update(updateWrapper);
        }
        return null;
    }

    private String caseToDynAtt(SmcOlMtr olMtr) {
        if (olMtr.getTaskStatus() != OL_MTR_STATUS_NOT_START)
            return "不匹配状态: 未启动";
        if (new Date().before(olMtr.getStartTime()))
            return "考试时间未到";
        String dynAttId = olMtr.getDynAttId();
        if (dynAttId != null) {
            SmcDynAtt dynAtt = smcDynAttService.getById(dynAttId);
            if (dynAtt != null)
                return "无法重新绑定动态点名任务";
        }
        SmcDynAtt dynAtt = new SmcDynAtt();

        dynAtt.setCreateBy(olMtr.getCreateBy());
        dynAtt.setCreateTime(olMtr.getCreateTime());
        dynAtt.setSysOrgCode(olMtr.getSysOrgCode());
        dynAtt.setUpdateBy(olMtr.getUpdateBy());
        dynAtt.setUpdateTime(olMtr.getUpdateTime());

        //填充业务字段
        dynAtt.setClassId(olMtr.getClassId());
        //添加任务
        String result = smcDynAttTaskService.save(dynAtt);
        if (result != null)
            return result;
        //绑定任务
        UpdateWrapper<SmcOlMtr> updateWrapper = new UpdateWrapper<>();
        updateWrapper.set("dyn_att_id", dynAtt.getId());
        updateWrapper.eq("id", olMtr.getId());
        smcOlMtrService.update(updateWrapper);
        olMtr.setDynAttId(dynAtt.getId());
        return null;
    }

    private String caseToChtDet(SmcOlMtr olMtr) {
        if (olMtr.getTaskStatus() != OL_MTR_STATUS_DYN_ATT)
            return "不匹配状态: 动态点名";
        if (new Date().before(olMtr.getStartTime()))
            return "考试时间未到";
        String chtDetId = olMtr.getChtDetId();
        if (chtDetId == null) {
            String result = createAndBindChtDet(olMtr);
            if (result != null)
                return result;
            chtDetId = olMtr.getChtDetId();
            if (chtDetId == null)
                return "无法绑定作弊检测任务";
            UpdateWrapper<SmcOlMtr> updateWrapper = new UpdateWrapper<>();
            updateWrapper.set("cht_det_id", chtDetId);
            updateWrapper.eq("id", olMtr.getId());
            smcOlMtrService.update(updateWrapper);
        }

        SmcChtDet chtDet = smcChtDetService.getById(chtDetId);
        if (chtDet == null)
            return "尝试后未绑定作弊检测任务";
        //结束动态点名任务
        smcDynAttTaskService.stopById(olMtr.getDynAttId());
        return null;
    }

    private String caseToFinish(SmcOlMtr olMtr) {
        if (olMtr.getTaskStatus() != OL_MTR_STATUS_CHT_DET)
            return "不匹配状态: 作弊检测";
        if (new Date().before(olMtr.getEndTime()))
            return "考试时间未结束";
        smcChtDetTaskService.stopById(olMtr.getChtDetId());
        return null;
    }


    @Override
    public void removeById(String taskId) {
        SmcOlMtr olMtr = smcOlMtrService.getById(taskId);
        if (olMtr == null)
            return;
        String chtDetId = olMtr.getChtDetId();
        if (chtDetId != null) {
            smcChtDetTaskService.removeById(chtDetId);
        }
        String dynAttId = olMtr.getDynAttId();
        if (dynAttId != null) {
            smcDynAttTaskService.removeById(dynAttId);
        }

        smcOlMtrService.removeById(taskId);

    }

    @Override
    public String save(SmcOlMtr smcOlMtr) {
        if (smcOlMtr.getEndTime().before(new Date())) {
            return "无法添加过期的任务";  //任务已经结束
        }
        if (smcOlMtr.getVideoSrcId() == null) {
            String classId = smcOlMtr.getClassId();
            if (classId == null) {
                return "缺少绑定的教室";
            }
            SmcClass smcClass = smcClassService.getById(classId);
            if (smcClass == null) {
                return "绑定的教室错误";
            }
            String videoSrcId = smcClass.getVideoSrcId();
            if (videoSrcId == null) {
                return "所绑定的教室无视频源";
            }

            smcOlMtr.setVideoSrcId(videoSrcId);
        }

        String result = createAndBindChtDet(smcOlMtr);
        if (result != null) {
            return result;
        }
        smcOlMtrService.save(smcOlMtr);
        generateTaskNameIfEmpty(smcOlMtr);
        return null;
    }

    private String createAndBindChtDet(SmcOlMtr smcOlMtr) {
        String chtDetId = smcOlMtr.getChtDetId();
        if (chtDetId != null) {
            SmcChtDet chtDet = smcChtDetService.getById(chtDetId);
            if (chtDet != null) {
                return "无法重新绑定检测任务";
            }
        }
        SmcChtDet chtDet = new SmcChtDet();

        chtDet.setCreateBy(smcOlMtr.getCreateBy());
        chtDet.setCreateTime(smcOlMtr.getCreateTime());
        chtDet.setSysOrgCode(smcOlMtr.getSysOrgCode());
        chtDet.setUpdateBy(smcOlMtr.getUpdateBy());
        chtDet.setUpdateTime(smcOlMtr.getUpdateTime());

        //填充业务字段
        chtDet.setStartTime(smcOlMtr.getStartTime());
        chtDet.setEndTime(smcOlMtr.getEndTime());
        chtDet.setVideoSrcId(smcOlMtr.getVideoSrcId());
        //添加任务
        String result = smcChtDetTaskService.save(chtDet);
        if (result != null)
            return result;
        //绑定任务
        smcOlMtr.setChtDetId(chtDet.getId());

        return null;

    }


    private void generateTaskNameIfEmpty(SmcOlMtr olMtr) {
        String taskName = olMtr.getTaskName();
        if (taskName == null || "".equals(taskName)) {
            String videoName = smcVideoSrcService.getById(olMtr.getVideoSrcId()).getVideoName();
            String newTaskName = String.format("在线监考(%s)%s", videoName, DateUtils.now());
            UpdateWrapper<SmcOlMtr> updateWrapper = new UpdateWrapper<>();
            updateWrapper.eq("id", olMtr.getId());
            updateWrapper.set("task_name", newTaskName);
            smcOlMtrService.update(updateWrapper);
        }
    }
}
