<template>
  <div>
    <a-card>
      <smc-ol-mtr-not-start
        v-if="taskStatus===0"
        :task-obj="taskObj"
        @nextStep="nextStep"></smc-ol-mtr-not-start>
      <smc-ol-mtr-dyn-att
        v-if="taskStatus===1"
        :task-obj="taskObj"
        @nextStep="nextStep"></smc-ol-mtr-dyn-att>
      <smc-ol-mtr-cht-det
        v-if="taskStatus===2"
        :task-obj="taskObj"
        @nextStep="nextStep"></smc-ol-mtr-cht-det>
      <smc-ol-mtr-finish
        v-if="taskStatus===3"
        :task-obj="taskObj"
        @nextStep="nextStep"></smc-ol-mtr-finish>
    </a-card>
    <a-card :bordered="false">
      <a-steps :current="taskStatus" size="small">
        <a-step title="考试准备中"/>
        <a-step title="人员认证中"/>
        <a-step title="监考中"/>
        <a-step title="考试结束"/>
      </a-steps>
    </a-card>
  </div>
</template>

<script>
import SmcOlMtrNotStart from "@views/smc/ol_mtr/modules/SmcOlMtrNotStart";
import SmcOlMtrDynAtt from "@views/smc/ol_mtr/modules/SmcOlMtrDynAtt";
import SmcOlMtrChtDet from "@views/smc/ol_mtr/modules/SmcOlMtrChtDet";
import SmcOlMtrFinish from "@views/smc/ol_mtr/modules/SmcOlMtrFinish";

import {getAction} from '@api/manage'

export default {
  name: "SmcOlMtr",
  components: {SmcOlMtrFinish, SmcOlMtrChtDet, SmcOlMtrDynAtt, SmcOlMtrNotStart},
  data() {
    return {
      taskId: this.$route.params.task_id || "",
      taskStatus: 0,
      taskObj: null,
      url: {
        nextStep: "/smc/smcOlMtrExtra/nextStep",
        queryById: "/smc/smcOlMtr/queryById"
      }
    }
  },
  created() {
    this.loadTask()
  },
  methods: {
    async loadTask() {
      let res = await getAction(this.url.queryById, {id: this.taskId})
      if (res) {
        if (res.success) {
          this.taskObj = res.result
          this.taskStatus = this.taskObj.taskStatus
        } else {
          this.$message.warning(res.message)
        }
      } else {
        this.$message.warning("找不到任务id " + this.taskId)
      }
    },
    async nextStep() {
      if (this.taskStatus >= 3)
        return

      let res = await getAction(this.url.nextStep, {id: this.taskId})
      if (res) {
        if (res.success) {
          this.$message.success(res.message)
          await this.loadTask()
        } else {
          this.$message.warning(res.message)
        }
      } else {
        this.$message.warning("找不到任务id " + this.taskId)
      }

    }
  }
}
</script>

<style scoped>

</style>