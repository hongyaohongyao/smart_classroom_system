<template>
  <a-spin :spinning="confirmLoading">
    <j-form-container :disabled="formDisabled">
      <a-form-model ref="form" :model="model" :rules="validatorRules" slot="detail">
        <a-row>
          <a-col :span="24">
            <a-form-model-item label="视频名称" :labelCol="labelCol" :wrapperCol="wrapperCol" prop="videoName">
              <a-input v-model="model.videoName" placeholder="请输入视频名称"  ></a-input>
            </a-form-model-item>
          </a-col>
          <a-col :span="24">
            <a-form-model-item label="视频源地址" :labelCol="labelCol" :wrapperCol="wrapperCol" prop="videoUrl">
              <a-input v-model="model.videoUrl" placeholder="请输入视频源地址"  ></a-input>
            </a-form-model-item>
          </a-col>
          <a-col :span="24">
            <a-form-model-item label="视频源类型" :labelCol="labelCol" :wrapperCol="wrapperCol" prop="videoType">
              <j-dict-select-tag type="list" v-model="model.videoType" dictCode="video_src_type" placeholder="请选择视频源类型" />
            </a-form-model-item>
          </a-col>
          <a-col :span="24">
            <a-form-model-item label="附加信息" :labelCol="labelCol" :wrapperCol="wrapperCol" prop="videoExtra">
              <a-textarea v-model="model.videoExtra" rows="4" placeholder="请输入附加信息" />
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
    name: 'SmcVideoSrcForm',
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
           videoName: [
              { required: true, message: '请输入视频名称!'},
           ],
           videoUrl: [
              { required: true, message: '请输入视频源地址!'},
           ],
           videoType: [
              { required: true, message: '请输入视频源类型!'},
           ],
        },
        url: {
          add: "/smc/smcVideoSrc/add",
          edit: "/smc/smcVideoSrc/edit",
          queryById: "/smc/smcVideoSrc/queryById"
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