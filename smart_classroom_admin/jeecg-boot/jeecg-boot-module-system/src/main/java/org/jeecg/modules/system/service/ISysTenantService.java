package org.jeecg.modules.system.service;

import com.baomidou.mybatisplus.extension.service.IService;
import org.jeecg.modules.system.entity.SysTenant;

import java.util.Collection;
import java.util.List;

public interface ISysTenantService extends IService<SysTenant> {

    /**
     * 查询有效的租户
     *
     * @param idList
     * @return
     */
    List<SysTenant> queryEffectiveTenant(Collection<String> idList);

    /**
     * 返回某个租户被多少个用户引用了
     *
     * @param id
     * @return
     */
    int countUserLinkTenant(String id);

    /**
     * 根据ID删除租户，会判断是否已被引用
     *
     * @param id
     * @return
     */
    boolean removeTenantById(String id);

}
