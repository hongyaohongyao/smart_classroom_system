package org.jeecg.modules.smc.service.impl;

import com.alibaba.fastjson.JSONValidator;
import org.jeecg.modules.smc.entity.SmcAiServer;
import org.jeecg.modules.smc.entity.SmcFaceReg;
import org.jeecg.modules.smc.service.ISmcFaceRegService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Service;
import org.springframework.web.client.RestTemplate;
import org.jeecg.modules.smc.service.ISmcAiServerManagerService;
import org.jeecg.modules.smc.service.ISmcFaceEncoderService;

@Service
public class SmcFaceEncoderServiceImpl implements ISmcFaceEncoderService {
    @Autowired
    ISmcFaceRegService smcFaceRegService;
    @Autowired
    RestTemplate restTemplate;
    @Autowired
    ISmcAiServerManagerService smcAiServerManagerService;

    @Override
    public String encodeAndSaveFace(SmcFaceReg smcFaceReg) {
        String faceImg = smcFaceReg.getFaceImg();
        if (faceImg == null || faceImg.length() == 0) {
            return "need face image";
        }
        boolean cond = smcFaceReg.getId() == null;
        cond = cond || !faceImg.equals(smcFaceRegService.getById(smcFaceReg.getId()).getFaceImg());
        if (cond) {
    SmcAiServer aiServer = smcAiServerManagerService.dispatchServer(smcFaceReg);
    String restUrl = String.format("%s/reg_face?url={1}", aiServer.getServerUrl());
    ResponseEntity<String> result = restTemplate.getForEntity(restUrl, String.class, faceImg);
    String encode = result.getBody();
            if (encode == null || encode.length() == 0) {
                return "encoding failed";
            }
            System.out.println(encode);
            if (JSONValidator.from(encode).getType() != JSONValidator.Type.Array) {
                return "encoding result wrong";
            }
            smcFaceReg.setFaceEncode(encode);
        }
        smcFaceRegService.saveOrUpdate(smcFaceReg);
        return null;
    }
}
