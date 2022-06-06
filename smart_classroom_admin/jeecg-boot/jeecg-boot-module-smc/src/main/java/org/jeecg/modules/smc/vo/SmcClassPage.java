package org.jeecg.modules.smc.vo;

import java.util.List;
import org.jeecg.modules.smc.entity.SmcClass;
import org.jeecg.modules.smc.entity.SmcClassMem;
import lombok.Data;
import org.jeecgframework.poi.excel.annotation.Excel;
import org.jeecgframework.poi.excel.annotation.ExcelEntity;
import org.jeecgframework.poi.excel.annotation.ExcelCollection;
import com.fasterxml.jackson.annotation.JsonFormat;
import org.springframework.format.annotation.DateTimeFormat;
import java.util.Date;
import org.jeecg.common.aspect.annotation.Dict;
import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;

/**
 * @Description: 智慧教室班级
 * @Author: jeecg-boot
 * @Date:   2022-03-11
 * @Version: V1.0
 */
@Data
@ApiModel(value="smc_classPage对象", description="智慧教室班级")
public class SmcClassPage {

	/**主键*/
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
	/**班级名称*/
	@Excel(name = "班级名称", width = 15)
	@ApiModelProperty(value = "班级名称")
    private java.lang.String className;
	/**摄像头*/
	@Excel(name = "摄像头", width = 15, dictTable = "smc_video_src", dicText = "video_name", dicCode = "id")
    @Dict(dictTable = "smc_video_src", dicText = "video_name", dicCode = "id")
	@ApiModelProperty(value = "摄像头")
    private java.lang.String videoSrcId;

	@ExcelCollection(name="智慧教室成员")
	@ApiModelProperty(value = "智慧教室成员")
	private List<SmcClassMem> smcClassMemList;

}
