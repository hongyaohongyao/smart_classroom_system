<template>
  <div>
    <smc-home v-if="indexStyle==1"></smc-home>
    <index-chart v-if="indexStyle==2"></index-chart>
    <index-bdc v-if="indexStyle==3"></index-bdc>
    <index-task v-if="indexStyle==4"></index-task>
    <div style="width: 100%;text-align: right;margin-top: 20px">
      请选择首页样式：
      <a-radio-group v-model="indexStyle">
        <a-radio :value="1">智慧教室监考系统首页</a-radio>
        <a-radio :value="2">统计图表</a-radio>
        <a-radio :value="3">统计图表2</a-radio>
        <a-radio :value="4">任务表格</a-radio>
      </a-radio-group>
    </div>
  </div>
</template>

<script>
import IndexChart from './IndexChart'
import IndexTask from "./IndexTask"
import IndexBdc from './IndexBdc'
import SmcHome from "@views/smc/SmcHome";

export default {
  name: "Analysis",
  components: {
    SmcHome,
    IndexChart,
    IndexTask,
    IndexBdc
  },
  data() {
    return {
      indexStyle: 1
    }
  },
  created() {

  },
  methods: {}
}
</script>