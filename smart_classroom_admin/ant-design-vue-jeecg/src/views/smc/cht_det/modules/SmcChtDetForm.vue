<template>
  <a-spin :spinning="confirmLoading">
    <j-form-container :disabled="formDisabled">
      <a-form-model ref="form" :model="model" :rules="validatorRules" slot="detail">
        <a-row>
          <a-col :span="24">
            <a-form-model-item label="任务名称" :labelCol="labelCol" :wrapperCol="wrapperCol" prop="taskName">
              <a-input v-model="model.taskName" placeholder="任务名称自动生成" disabled></a-input>
            </a-form-model-item>
          </a-col>
          <a-col :span="24">
            <a-form-model-item label="视频源" :labelCol="labelCol" :wrapperCol="wrapperCol" prop="videoSrcId">
              <j-search-select-tag v-model="model.videoSrcId" dict="smc_video_src,video_name,id"  />
            </a-form-model-item>
          </a-col>
          <a-col :span="24">
            <a-form-model-item label="启动时间" :labelCol="labelCol" :wrapperCol="wrapperCol" prop="startTime">
              <j-date placeholder="请选择启动时间" v-model="model.startTime" :show-time="true"
                      date-format="YYYY-MM-DD HH:mm:ss" style="width: 100%"
                      @change="taskTimeChange"/>
            </a-form-model-item>
          </a-col>
          <a-col :span="24">
            <a-form-item label="考试时长" :labelCol="labelCol" :wrapperCol="wrapperCol">
              <j-time placeholder="请输入考试时长" v-model="taskTime" @change="taskTimeChange">

              </j-time>
            </a-form-item>
          </a-col>
          <a-col :span="24">
            <a-form-model-item label="结束时间" :labelCol="labelCol" :wrapperCol="wrapperCol" prop="endTime">
              <j-date placeholder="请选择结束时间" v-model="model.endTime" :show-time="true" date-format="YYYY-MM-DD HH:mm:ss"
                      style="width: 100%"
                      @change="endTimeChange"
              />
            </a-form-model-item>
          </a-col>
          <a-col :span="24">
            <a-form-model-item label="分配服务器" :labelCol="labelCol" :wrapperCol="wrapperCol" prop="aiServerId">
              <j-dict-select-tag type="list" v-model="model.aiServerId" dictCode="smc_ai_server,server_name,id"
                                 placeholder="自动分配服务器" disabled/>
            </a-form-model-item>
          </a-col>
        </a-row>
      </a-form-model>
    </j-form-container>
  </a-spin>
</template>

<script>

import {httpAction} from '@api/manage'
import {formatDate, randomNumber} from '@/utils/util'
import moment from "moment";

export default {
  name: 'SmcChtDetForm',
  components: {},
  props: {
    //表单禁用
    disabled: {
      type: Boolean,
      default: false,
      required: false
    }
  },
  data() {
    return {
      model: {},
      labelCol: {
        xs: {span: 24},
        sm: {span: 5},
      },
      taskTime: "00:00:00",
      wrapperCol: {
        xs: {span: 24},
        sm: {span: 16},
      },
      confirmLoading: false,
      validatorRules: {
        videoSrcId: [
          {required: true, message: '请输入视频源!'},
        ],
        startTime: [
          {required: true, message: '请输入启动时间!'},
        ],
        endTime: [
          {required: true, message: '请输入结束时间!'},
        ],
      },
      url: {
        add: "/smc/smcChtDet/add",
        edit: "/smc/smcChtDet/edit",
        queryById: "/smc/smcChtDet/queryById"
      }
    }
  },
  computed: {
    formDisabled() {
      return this.disabled
    },
  },
  created() {
    //备份model原始值
    this.modelDefault = JSON.parse(JSON.stringify(this.model));
    let that = this
    setTimeout(() => {
      that.endTimeChange()
    }, 200)
  },
  methods: {
    taskTimeChange() {
      if (!!this.taskTime && this.model.startTime) {
        this.model.endTime = moment(this.model.startTime, "YYYY-MM-DD HH:mm:ss")
          .add(moment.duration(this.taskTime))
          .format("YYYY-MM-DD HH:mm:ss")
        console.log(this.model.endTime)
      }
    },
    endTimeChange() {
      if (!!this.model.endTime && !!this.model.startTime) {
        let startTime = moment(this.model.startTime, "YYYY-MM-DD HH:mm:ss")
        let endTime = moment(this.model.endTime, "YYYY-MM-DD HH:mm:ss")
        let diff = moment.duration(endTime.diff(startTime))
        if (diff.as("hours") >= 24 || diff.as("seconds") < 0) {
          this.$message.warning("时间应大于0小于24小时")
          this.taskTimeChange()
        } else {
          let v = diff.hours().toString().padStart(2, "0")
            + ":"
            + diff.minutes().toString().padStart(2, "0")
            + ":"
            + diff.seconds().toString().padStart(2, "0")
          this.taskTime = v
        }
      }
    },
    add() {
      this.edit(this.modelDefault);
    },
    edit(record) {
      this.model = Object.assign({}, record);
      this.visible = true;
    },
    submitForm() {
      const that = this;
      // 触发表单验证
      this.$refs.form.validate(valid => {
        if (valid) {
          that.confirmLoading = true;
          let httpurl = '';
          let method = '';
          if (!this.model.id) {
            httpurl += this.url.add;
            method = 'post';
          } else {
            httpurl += this.url.edit;
            method = 'put';
          }
          httpAction(httpurl, this.model, method).then((res) => {
            if (res.success) {
              that.$message.success(res.message);
              that.$emit('ok');
            } else {
              that.$message.warning(res.message);
            }
          }).finally(() => {
            that.confirmLoading = false;
          })
        }

      })
    },
  }
}
</script>