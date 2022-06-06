package org.jeecg.modules.smc.mapper;

import java.util.List;
import org.jeecg.modules.smc.entity.SmcClassMem;
import com.baomidou.mybatisplus.core.mapper.BaseMapper;
import org.apache.ibatis.annotations.Param;

/**
 * @Description: 智慧教室成员
 * @Author: jeecg-boot
 * @Date:   2022-03-11
 * @Version: V1.0
 */
public interface SmcClassMemMapper extends BaseMapper<SmcClassMem> {

	public boolean deleteByMainId(@Param("mainId") String mainId);
    
	public List<SmcClassMem> selectByMainId(@Param("mainId") String mainId);
}
