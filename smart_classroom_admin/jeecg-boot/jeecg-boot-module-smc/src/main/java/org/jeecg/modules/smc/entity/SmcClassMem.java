package org.jeecg.modules.smc.entity;

import java.io.Serializable;
import com.baomidou.mybatisplus.annotation.IdType;
import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;
import lombok.Data;
import com.fasterxml.jackson.annotation.JsonFormat;
import org.springframework.format.annotation.DateTimeFormat;
import org.jeecgframework.poi.excel.annotation.Excel;
import java.util.Date;
import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import java.io.UnsupportedEncodingException;

/**
 * @Description: 智慧教室成员
 * @Author: jeecg-boot
 * @Date:   2022-03-11
 * @Version: V1.0
 */
@ApiModel(value="smc_class_mem对象", description="智慧教室成员")
@Data
@TableName("smc_class_mem")
public class SmcClassMem implements Serializable {
    private static final long serialVersionUID = 1L;

	/**主键*/
	@TableId(type = IdType.ASSIGN_ID)
    @ApiModelProperty(value = "主键")
    private java.lang.String id;
	/**创建人*/
    @ApiModelProperty(value = "创建人")
    private java.lang.String createBy;
	/**创建日期*/
	@JsonFormat(timezone = "GMT+8",pattern = "yyyy-MM-dd HH:mm:ss")
    @DateTimeFormat(pattern="yyyy-MM-dd HH:mm:ss")
    @ApiModelProperty(value = "创建日期")
    private java.util.Date createTime;
	/**更新人*/
    @ApiModelProperty(value = "更新人")
    private java.lang.String updateBy;
	/**更新日期*/
	@JsonFormat(timezone = "GMT+8",pattern = "yyyy-MM-dd HH:mm:ss")
    @DateTimeFormat(pattern="yyyy-MM-dd HH:mm:ss")
    @ApiModelProperty(value = "更新日期")
    private java.util.Date updateTime;
	/**所属部门*/
    @ApiModelProperty(value = "所属部门")
    private java.lang.String sysOrgCode;
	/**成员名称*/
	@Excel(name = "成员名称", width = 15)
    @ApiModelProperty(value = "成员名称")
    private java.lang.String memName;
	/**关联人脸*/
	@Excel(name = "关联人脸", width = 15, dictTable = "smc_face_reg", dicText = "face_name", dicCode = "id")
    @ApiModelProperty(value = "关联人脸")
    private java.lang.String faceRegId;
	/**所属教室*/
    @ApiModelProperty(value = "所属教室")
    private java.lang.String classId;
	/**所属用户*/
	@Excel(name = "所属用户", width = 15, dictTable = "sys_user", dicText = "username", dicCode = "id")
    @ApiModelProperty(value = "所属用户")
    private java.lang.String userId;
}
