package org.jeecg.modules.smc.entity;

import java.io.Serializable;
import java.io.UnsupportedEncodingException;
import java.util.Date;
import java.math.BigDecimal;
import com.baomidou.mybatisplus.annotation.IdType;
import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;
import lombok.Data;
import com.fasterxml.jackson.annotation.JsonFormat;
import org.springframework.format.annotation.DateTimeFormat;
import org.jeecgframework.poi.excel.annotation.Excel;
import org.jeecg.common.aspect.annotation.Dict;
import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.EqualsAndHashCode;
import lombok.experimental.Accessors;

/**
 * @Description: 智慧教室在线监考
 * @Author: jeecg-boot
 * @Date:   2022-04-16
 * @Version: V1.0
 */
@Data
@TableName("smc_ol_mtr")
@Accessors(chain = true)
@EqualsAndHashCode(callSuper = false)
@ApiModel(value="smc_ol_mtr对象", description="智慧教室在线监考")
public class SmcOlMtr implements Serializable {
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
	/**任务名称*/
	@Excel(name = "任务名称", width = 15)
    @ApiModelProperty(value = "任务名称")
    private java.lang.String taskName;
	/**开考时间*/
	@Excel(name = "开考时间", width = 20, format = "yyyy-MM-dd HH:mm:ss")
	@JsonFormat(timezone = "GMT+8",pattern = "yyyy-MM-dd HH:mm:ss")
    @DateTimeFormat(pattern="yyyy-MM-dd HH:mm:ss")
    @ApiModelProperty(value = "开考时间")
    private java.util.Date startTime;
	/**考试结束时间*/
	@Excel(name = "考试结束时间", width = 20, format = "yyyy-MM-dd HH:mm:ss")
	@JsonFormat(timezone = "GMT+8",pattern = "yyyy-MM-dd HH:mm:ss")
    @DateTimeFormat(pattern="yyyy-MM-dd HH:mm:ss")
    @ApiModelProperty(value = "结束时间")
    private java.util.Date endTime;
	/**关联考场*/
	@Excel(name = "关联考场", width = 15, dictTable = "smc_class", dicText = "class_name", dicCode = "id")
	@Dict(dictTable = "smc_class", dicText = "class_name", dicCode = "id")
    @ApiModelProperty(value = "关联考场")
    private java.lang.String classId;
	/**考场监控*/
	@Excel(name = "考场监控", width = 15, dictTable = "smc_video_src", dicText = "video_name", dicCode = "id")
	@Dict(dictTable = "smc_video_src", dicText = "video_name", dicCode = "id")
    @ApiModelProperty(value = "考场监控")
    private java.lang.String videoSrcId;
	/**关联动态点名任务*/
	@Excel(name = "关联动态点名任务", width = 15)
    @ApiModelProperty(value = "关联动态点名任务")
    private java.lang.String dynAttId;
	/**关联作弊检测任务*/
	@Excel(name = "关联作弊检测任务", width = 15)
    @ApiModelProperty(value = "关联作弊检测任务")
    private java.lang.String chtDetId;
	/**任务状态*/
	@Excel(name = "任务状态", width = 15, dicCode = "ol_mtr_status")
	@Dict(dicCode = "ol_mtr_status")
    @ApiModelProperty(value = "任务状态")
    private java.lang.Integer taskStatus;
	/**其他信息*/
	@Excel(name = "其他信息", width = 15)
    @ApiModelProperty(value = "其他信息")
    private java.lang.String other;
}
