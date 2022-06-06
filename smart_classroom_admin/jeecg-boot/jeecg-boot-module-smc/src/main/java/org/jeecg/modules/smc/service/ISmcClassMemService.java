package org.jeecg.modules.smc.service;

import org.jeecg.modules.smc.entity.SmcClassMem;
import com.baomidou.mybatisplus.extension.service.IService;
import java.util.List;

/**
 * @Description: 智慧教室成员
 * @Author: jeecg-boot
 * @Date:   2022-03-11
 * @Version: V1.0
 */
public interface ISmcClassMemService extends IService<SmcClassMem> {

	public List<SmcClassMem> selectByMainId(String mainId);
}
