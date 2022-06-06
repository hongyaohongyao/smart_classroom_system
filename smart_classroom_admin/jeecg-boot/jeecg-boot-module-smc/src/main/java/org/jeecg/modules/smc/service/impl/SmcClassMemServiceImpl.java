package org.jeecg.modules.smc.service.impl;

import org.jeecg.modules.smc.entity.SmcClassMem;
import org.jeecg.modules.smc.mapper.SmcClassMemMapper;
import org.jeecg.modules.smc.service.ISmcClassMemService;
import org.springframework.stereotype.Service;
import java.util.List;
import com.baomidou.mybatisplus.extension.service.impl.ServiceImpl;
import org.springframework.beans.factory.annotation.Autowired;

/**
 * @Description: 智慧教室成员
 * @Author: jeecg-boot
 * @Date:   2022-03-11
 * @Version: V1.0
 */
@Service
public class SmcClassMemServiceImpl extends ServiceImpl<SmcClassMemMapper, SmcClassMem> implements ISmcClassMemService {
	
	@Autowired
	private SmcClassMemMapper smcClassMemMapper;
	
	@Override
	public List<SmcClassMem> selectByMainId(String mainId) {
		return smcClassMemMapper.selectByMainId(mainId);
	}
}
