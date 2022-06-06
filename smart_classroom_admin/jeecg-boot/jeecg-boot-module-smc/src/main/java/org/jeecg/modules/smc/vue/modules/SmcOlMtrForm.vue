<template>
  <a-spin :spinning="confirmLoading">
    <j-form-container :disabled="formDisabled">
      <a-form-model ref="form" :model="model" :rules="validatorRules" slot="detail">
        <a-row>
          <a-col :span="12">
            <a-form-model-item label="任务名称" :labelCol="labelCol" :wrapperCol="wrapperCol" prop="taskName">
              <a-input v-model="model.taskName" placeholder="请输入任务名称"  ></a-input>
            </a-form-model-item>
          </a-col>
          <a-col :span="12">
            <a-form-model-item label="开考时间" :labelCol="labelCol" :wrapperCol="wrapperCol" prop="startTime">
              <j-date placeholder="请选择开考时间"  v-model="model.startTime" :show-time="true" date-format="YYYY-MM-DD HH:mm:ss" style="width: 100%" />
            </a-form-model-item>
          </a-col>
          <a-col :span="12">
            <a-form-model-item label="考试结束时间" :labelCol="labelCol" :wrapperCol="wrapperCol" prop="endTime">
              <j-date placeholder="请选择考试结束时间"  v-model="model.endTime" :show-time="true" date-format="YYYY-MM-DD HH:mm:ss" style="width: 100%" />
            </a-form-model-item>
          </a-col>
          <a-col :span="12">
            <a-form-model-item label="关联考场" :labelCol="labelCol" :wrapperCol="wrapperCol" prop="classId">
              <j-search-select-tag v-model="model.classId" dict="smc_class,class_name,id"  />
            </a-form-model-item>
          </a-col>
          <a-col :span="12">
            <a-form-model-item label="考场监控" :labelCol="labelCol" :wrapperCol="wrapperCol" prop="videoSrcId">
              <j-search-select-tag v-model="model.videoSrcId" dict="smc_video_src,video_name,id"  />
            </a-form-model-item>
          </a-col>
          <a-col :span="12">
            <a-form-model-item label="任务状态" :labelCol="labelCol" :wrapperCol="wrapperCol" prop="taskStatus">
              <j-dict-select-tag type="list" v-model="model.taskStatus" dictCode="ol_mtr_status" placeholder="请选择任务状态" disabled/>
            </a-form-model-item>
          </a-col>
          <a-col :span="12">
            <a-form-model-item label="其他信息" :labelCol="labelCol" :wrapperCol="wrapperCol" prop="other">
              <a-textarea v-model="model.other" rows="4" placeholder="请输入其他信息" />
            </a-form-model-item>
          </a-col>
        </a-row>
      </a-form-model>
    </j-form-container>
  </a-spin>
</template>

<script>

  import { httpAction, getAction } from '@/api/manage'
  import { validateDuplicateValue } from '@/utils/util'

  export default {
    name: 'SmcOlMtrForm',
    components: {
    },
    props: {
      //表单禁用
      disabled: {
        type: Boolean,
        default: false,
        required: false
      }
    },
    data () {
      return {
        model:{
            taskStatus:0,
         },
        labelCol: {
          xs: { span: 24 },
          sm: { span: 5 },
        },
        wrapperCol: {
          xs: { span: 24 },
          sm: { span: 16 },
        },
        confirmLoading: false,
        validatorRules: {
           startTime: [
              { required: true, message: '请输入开考时间!'},
           ],
           endTime: [
              { required: true, message: '请输入考试结束时间!'},
           ],
           classId: [
              { required: true, message: '请输入关联考场!'},
           ],
           taskStatus: [
              { required: true, message: '请输入任务状态!'},
           ],
        },
        url: {
          add: "/smc/smcOlMtr/add",
          edit: "/smc/smcOlMtr/edit",
          queryById: "/smc/smcOlMtr/queryById"
        }
      }
    },
    computed: {
      formDisabled(){
        return this.disabled
      },
    },
    created () {
       //备份model原始值
      this.modelDefault = JSON.parse(JSON.stringify(this.model));
    },
    methods: {
      add () {
        this.edit(this.modelDefault);
      },
      edit (record) {
        this.model = Object.assign({}, record);
        this.visible = true;
      },
      submitForm () {
        const that = this;
        // 触发表单验证
        this.$refs.form.validate(valid => {
          if (valid) {
            that.confirmLoading = true;
            let httpurl = '';
            let method = '';
            if(!this.model.id){
              httpurl+=this.url.add;
              method = 'post';
            }else{
              httpurl+=this.url.edit;
               method = 'put';
            }
            httpAction(httpurl,this.model,method).then((res)=>{
              if(res.success){
                that.$message.success(res.message);
                that.$emit('ok');
              }else{
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