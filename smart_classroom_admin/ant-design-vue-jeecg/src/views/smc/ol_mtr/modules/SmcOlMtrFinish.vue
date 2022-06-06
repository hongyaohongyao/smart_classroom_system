<template>
  <div>
    <a-row>
      <a-col :span="8">
        <h1 class="taskTitle">考试结束</h1>
        <h1 style="text-align: center">开考时间: {{ startTime }}</h1>
        <h1 style="text-align: center">结束时间: {{ endTime }}</h1>
        <a-card title="考场信息">
          <a-row>
            <a-col :span="12"><b>考场:</b></a-col>
            <a-col :span="12">{{ classroomInfo.classroomName }}</a-col>
          </a-row>
          <a-row>
            <a-col :span="12"><b>考场监控:</b></a-col>
            <a-col :span="12">{{ classroomInfo.videoName }}</a-col>
          </a-row>
        </a-card>
      </a-col>
      <a-col :span="8">
        <a-row>

          <a-table :columns=" att_columns"
                   :data-source="att_list"
                   size="small"
                   :pagination="{ pageSize: 3,pageSizeOptions:['3','5'] }"
                   :scroll="{ y: 200 }"
                   :rowKey="r=>r.name">
            <template slot="htmlSlot" slot-scope="text">
              <div v-html="text"></div>
            </template>
            <template slot="imgSlot" slot-scope="text">
              <span v-if="!text" style="font-size: 12px;font-style: italic;">无图片</span>
              <img v-else :src="text" height="50px" alt=""
                   style="font-size: 12px;font-style: italic;"/>
            </template>
          </a-table>
        </a-row>
        <a-row>
          <a-table :columns="abs_columns"
                   :data-source="abs_list"
                   :pagination="{ pageSize: 5,pageSizeOptions:['3','5']}"
                   :scroll="{ y: 200 }"
                   size="small"
                   :rowKey="r=>r.name"/>
        </a-row>
      </a-col>
      <a-col :span="8">
        <smc-cht-det-snapshots :task-name="currentTaskName"
                               :task-obj="chtDetObj"></smc-cht-det-snapshots>
      </a-col>
    </a-row>
  </div>
</template>

<script>
import SmcChtDetSnapshots from "@views/smc/cht_det/modules/SmcChtDetSnapshots";
import {getAction} from "@api/manage";
import axios from "axios";

export default {
  name: "SmcOlMtrFinish",
  components: {SmcChtDetSnapshots},
  props: ["taskObj"],
  data() {
    return {

      classroomInfo: {
        classroomName: "",
        videoName: "",
      },

      currentTaskName: "",
      chtDetObj: {},
      chtDetId: "",
      dynAttId: "",
      startTime: "",
      endTime: "",
      url: {
        dynAttBaseUrl: "",
        dynAttDescUrl: "",
        queryChtDetById: "/smc/smcChtDet/queryById",
        queryDynAttById: "/smc/smcDynAtt/queryById",
        querySmcClassMemByMainId: "/smc/smcClass/querySmcClassMemByMainId",
        classroomInfo: "/smc/home/olMtrInfo"
      },
      //人员认证相关
      att_columns: [
        {
          title: '出勤人员',
          align: "center",
          dataIndex: 'name',
        },
        {
          title: '快照',
          dataIndex: 'img',
          align: "center",
          scopedSlots: {customRender: 'imgSlot'}
        }, {
          title: '置信度',
          align: "center",
          dataIndex: 'confidence',
        }
      ],
      abs_columns: [
        {
          title: '缺勤/未注册人员',
          align: "center",
          dataIndex: 'name',
        }
      ],
      att_list: [],
      abs_list: [],
      classMems: [],
      descTimeStamp: 0,
    }
  },
  created() {
    if (this.taskObj) {
      this.showTaskInfo()
      this.chtDetId = this.taskObj.chtDetId
      if (!!this.chtDetId) {
        this.init_start()
      }
      this.dynAttId = this.taskObj.dynAttId
      if (!!this.dynAttId) {
        let baseUrl = "/file_sys/snapshots/dynatt_" +
          this.dynAttId + "/"
        this.url.dynAttBaseUrl = baseUrl
        this.url.dynAttDescUrl = baseUrl + ".desc"
        this.updateList()
      }
    }
  },
  methods: {
    showTaskInfo() {
      this.startTime = this.taskObj.startTime
      this.endTime = this.taskObj.endTime
      let that = this
      getAction(this.url.classroomInfo, {id: this.taskObj.id}).then(res => {
        if (res.success) {
          that.classroomInfo = res.result
        } else {
          this.$message.warning("考场信息查询错误")
        }
      })
    },
    init_start() {
      const that = this
      const zg = /^(?![0-9]+$)$/;
      if (zg.test(this.chtDetId)) {
        this.$message.warning("任务id由数字组成")
        return;
      }
      getAction(this.url.queryChtDetById, {id: this.chtDetId}).then(async (resp) => {
        console.log(resp)
        if (resp.success) {
          that.chtDetObj = resp.result
          that.currentTaskName = that.chtDetId;
        } else {
          that.$message.warning("找不到监考任务" + that.chtDetId)
        }
      })
    },
    async updateList() {
      //获取班级数据
      this.dynAttInfo = (await getAction(this.url.queryDynAttById,
        {id: this.dynAttId})).result
      if (!!!this.dynAttInfo) {
        return;
      }
      this.classMems = (await getAction(this.url.querySmcClassMemByMainId,
        {id: this.dynAttInfo.classId})).result
      if (!!!this.classMems) {
        return
      }
      //轮询更新表格
      let that = this
      axios.get(that.url.dynAttDescUrl).then(function (resp) {
        resp = resp.data
        if (resp.timestamp !== that.descTimeStamp) {
          that.setList(resp)
          that.descTimeStamp = resp.timestamp
        }
      })
    },
    setList(desc) {
      let att_list = []
      let abs_list = []
      let attendace = desc.attendance
      let that = this
      this.classMems.forEach(function (f, i) {
        if (attendace[f.id]) {
          let matchInfo = attendace[f.id]
          att_list.push({
            name: f.memName,
            img: that.url.dynAttBaseUrl + matchInfo.idx + ".jpg?t=" + matchInfo.timestamp.toString(),
            confidence: (matchInfo.confidence * 100).toFixed(2) + "%"
          })
        } else {
          abs_list.push({name: f.memName})
        }
      })
      this.att_list = att_list
      this.abs_list = abs_list
    },
  }
}
</script>

<style scoped>
.taskTitle {
  text-align: center;
  font-size: 5vw;
}
</style>