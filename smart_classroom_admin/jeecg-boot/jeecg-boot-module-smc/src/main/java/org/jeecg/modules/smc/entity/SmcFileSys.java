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
 * @Description: 检测结果保存的文件系统
 * @Author: jeecg-boot
 * @Date:   2022-02-10
 * @Version: V1.0
 */
@Data
@TableName("smc_file_sys")
@Accessors(chain = true)
@EqualsAndHashCode(callSuper = false)
@ApiModel(value="smc_file_sys对象", description="检测结果保存的文件系统")
public class SmcFileSys implements Serializable {
    private static final long serialVersionUID = 1L;

	/**主键*/
	@TableId(type = IdType.ASSIGN_ID)
    @ApiModelProperty(value = "主键")
    private String id;
	/**创建人*/
    @ApiModelProperty(value = "创建人")
    private String createBy;
	/**创建日期*/
	@JsonFormat(timezone = "GMT+8",pattern = "yyyy-MM-dd HH:mm:ss")
    @DateTimeFormat(pattern="yyyy-MM-dd HH:mm:ss")
    @ApiModelProperty(value = "创建日期")
    private Date createTime;
	/**更新人*/
    @ApiModelProperty(value = "更新人")
    private String updateBy;
	/**更新日期*/
	@JsonFormat(timezone = "GMT+8",pattern = "yyyy-MM-dd HH:mm:ss")
    @DateTimeFormat(pattern="yyyy-MM-dd HH:mm:ss")
    @ApiModelProperty(value = "更新日期")
    private Date updateTime;
	/**所属部门*/
    @ApiModelProperty(value = "所属部门")
    private String sysOrgCode;
	/**文件系统名称*/
	@Excel(name = "文件系统名称", width = 15)
    @ApiModelProperty(value = "文件系统名称")
    private String fileSysName;
	/**文件系统地址*/
	@Excel(name = "文件系统地址", width = 15)
    @ApiModelProperty(value = "文件系统地址")
    private String fileSysUrl;
	/**文件系统类型*/
	@Excel(name = "文件系统类型", width = 15, dicCode = "file_sys_type")
	@Dict(dicCode = "file_sys_type")
    @ApiModelProperty(value = "文件系统类型")
    private Integer fileSysType;
}
