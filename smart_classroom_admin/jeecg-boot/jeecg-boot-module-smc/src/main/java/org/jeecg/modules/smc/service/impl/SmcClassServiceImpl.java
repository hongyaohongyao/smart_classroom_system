package org.jeecg.modules.smc.service.impl;

import org.jeecg.modules.smc.entity.SmcClass;
import org.jeecg.modules.smc.entity.SmcClassMem;
import org.jeecg.modules.smc.mapper.SmcClassMemMapper;
import org.jeecg.modules.smc.mapper.SmcClassMapper;
import org.jeecg.modules.smc.service.ISmcClassService;
import org.springframework.stereotype.Service;
import com.baomidou.mybatisplus.extension.service.impl.ServiceImpl;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.transaction.annotation.Transactional;
import java.io.Serializable;
import java.util.List;
import java.util.Collection;

/**
 * @Description: 智慧教室班级
 * @Author: jeecg-boot
 * @Date:   2022-03-11
 * @Version: V1.0
 */
@Service
public class SmcClassServiceImpl extends ServiceImpl<SmcClassMapper, SmcClass> implements ISmcClassService {

	@Autowired
	private SmcClassMapper smcClassMapper;
	@Autowired
	private SmcClassMemMapper smcClassMemMapper;
	
	@Override
	@Transactional
	public void saveMain(SmcClass smcClass, List<SmcClassMem> smcClassMemList) {
		smcClassMapper.insert(smcClass);
		if(smcClassMemList!=null && smcClassMemList.size()>0) {
			for(SmcClassMem entity:smcClassMemList) {
				//外键设置
				entity.setClassId(smcClass.getId());
				smcClassMemMapper.insert(entity);
			}
		}
	}

	@Override
	@Transactional
	public void updateMain(SmcClass smcClass,List<SmcClassMem> smcClassMemList) {
		smcClassMapper.updateById(smcClass);
		
		//1.先删除子表数据
		smcClassMemMapper.deleteByMainId(smcClass.getId());
		
		//2.子表数据重新插入
		if(smcClassMemList!=null && smcClassMemList.size()>0) {
			for(SmcClassMem entity:smcClassMemList) {
				//外键设置
				entity.setClassId(smcClass.getId());
				smcClassMemMapper.insert(entity);
			}
		}
	}

	@Override
	@Transactional
	public void delMain(String id) {
		smcClassMemMapper.deleteByMainId(id);
		smcClassMapper.deleteById(id);
	}

	@Override
	@Transactional
	public void delBatchMain(Collection<? extends Serializable> idList) {
		for(Serializable id:idList) {
			smcClassMemMapper.deleteByMainId(id.toString());
			smcClassMapper.deleteById(id);
		}
	}
	
}
