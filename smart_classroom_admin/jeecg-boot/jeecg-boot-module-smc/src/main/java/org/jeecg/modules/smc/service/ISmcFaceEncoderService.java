package org.jeecg.modules.smc.service;

import org.jeecg.modules.smc.entity.SmcFaceReg;
import org.springframework.transaction.annotation.Transactional;

public interface ISmcFaceEncoderService {
    @Transactional(rollbackFor = {Exception.class})
    String encodeAndSaveFace(SmcFaceReg smcFaceReg);
}
