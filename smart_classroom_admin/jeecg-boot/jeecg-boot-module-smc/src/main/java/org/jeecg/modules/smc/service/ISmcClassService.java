package org.jeecg.modules.smc.service;

import org.jeecg.modules.smc.entity.SmcClassMem;
import org.jeecg.modules.smc.entity.SmcClass;
import com.baomidou.mybatisplus.extension.service.IService;
import java.io.Serializable;
import java.util.Collection;
import java.util.List;

/**
 * @Description: 智慧教室班级
 * @Author: jeecg-boot
 * @Date:   2022-03-11
 * @Version: V1.0
 */
public interface ISmcClassService extends IService<SmcClass> {

	/**
	 * 添加一对多
	 * 
	 */
	public void saveMain(SmcClass smcClass,List<SmcClassMem> smcClassMemList) ;
	
	/**
	 * 修改一对多
	 * 
	 */
	public void updateMain(SmcClass smcClass,List<SmcClassMem> smcClassMemList);
	
	/**
	 * 删除一对多
	 */
	public void delMain (String id);
	
	/**
	 * 批量删除一对多
	 */
	public void delBatchMain (Collection<? extends Serializable> idList);
	
}
