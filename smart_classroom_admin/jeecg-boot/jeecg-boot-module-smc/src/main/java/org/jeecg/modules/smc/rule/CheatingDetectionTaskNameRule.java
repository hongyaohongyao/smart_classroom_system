package org.jeecg.modules.smc.rule;

import com.alibaba.fastjson.JSONObject;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.math.RandomUtils;
import org.jeecg.common.handler.IFillRuleHandler;

import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * 填值规则Demo：作弊检测任务名称
 */
public class CheatingDetectionTaskNameRule implements IFillRuleHandler {

    @Override
    public Object execute(JSONObject params, JSONObject formData) {
        String value = "作弊检测(";
        //订单前缀默认为CN 如果规则参数不为空，则取自定义前缀
        String name = formData.getString("video_src_id");
        if (!StringUtils.isEmpty(name)) {
            value += name;
        } else {
            value += "unknown";
        }
        value += ")_";
        SimpleDateFormat format = new SimpleDateFormat("yyyyMMddHHmmss");
        int random = RandomUtils.nextInt(90) + 10;
        value = value + format.format(new Date()) + random;
        // 根据formData的值的不同，生成不同的订单号
        return value;
    }
}
