<template>
  <a-card>
    <div id="components-pagination-demo-mini">
      <a-pagination
        @showSizeChange="handleSizeChange"
        @change="handleCurrentChange"
        size="small"
        v-model="currentPage"
        :page-size-options="['3', '6', '9', '12', '15']"
        :page-size.sync="pageSize"
        show-size-changer show-quick-jumper
        :total="totalNum">
      </a-pagination>
    </div>
    <a-row :max-height="20">
      <a-col :span="8" v-for="detection in detectionsWin"
             :key="detection.key" align="center">
        <span>{{ detection.key + "." + chtTypeName[detection.chtType] }}</span><br/>
        <img style="font-size: 12px;font-style: italic;" height="75px"
             :src="detection.url" alt=""/>
      </a-col>
    </a-row>
  </a-card>
</template>

<script>
import axios from "axios";

export default {
  name: "SmcChtDetSnapshots",
  data() {
    return {
      chtTypeName: ["正常", "传递物品", "低头偷看", "东张西望"],
      updateMaxNumTimer: null,
      showSnapshot: false,
      totalNum: 0,//检测到作弊的帧的数量
      descBlockNums: [],
      detections: [],
      detectionsWin: [],
      currentPage: 1,
      pageSize: 12,
      url: {
        descUrlTemplate: ""
      }
    }
  },
  props: ["taskName", "taskObj"],
  created() {

  },
  destroyed() {
    clearInterval(this.updateMaxNumTimer)
  },
  methods: {
    updateMaxNum() {
      let this_ = this
      if (this.updateMaxNumTimer) {
        clearInterval(this.updateMaxNumTimer)
      }
      this.updateMaxNumTimer = setInterval(function () {
        axios.get(this_.url.descUrl).then(function (resp) {
          resp = resp.data
          if (this_.totalNum !== resp.curIdx) {
            console.log("update")
            let oldTotalNum = this_.totalNum
            console.log(this_.totalNum)
            console.log(resp.curIdx)
            console.log(this_.currentPage * this_.pageSize)
            this_.totalNum = resp.curIdx
            if (oldTotalNum < this_.currentPage * this_.pageSize) {
              this_.handleCurrentChange()
            }
          } else {
            console.log("waiting")
          }
        })
      }, 1000)
    },

    handleSizeChange(val) {
      this.handleCurrentChange()
    },
    async handleCurrentChange(val) {
      let this_ = this;
      let arr = [];
      let begin = (this.currentPage - 1) * this.pageSize
      let end = Math.min(this.currentPage * this.pageSize, this.totalNum)
      let j = 0;
      for (let i = begin; i < end; i++, j++) {
        if (this.detections[i] === undefined) {
          await axios.get(this_.url.baseUrl + Math.floor(i / 50) + ".desc").then(function (resp) {
            console.log(resp)
            resp = resp.data
            let detIdx = i;
            let len = resp.length;
            for (let k = i % 50; k < len; k++, detIdx++) {
              let detObj = resp[k]
              let imgId = detIdx
              this_.detections[detIdx] = {
                key: imgId,
                url: this_.url.baseUrl + imgId + ".jpg",
                chtType: detObj.chtType
              };
            }
          })
        }
        arr[j] = this.detections[i]
      }
      this.detectionsWin = arr
    }
  },
  computed: {
    taskName_() {
      return this.taskName || "";
    }
  },
  watch: {
    taskName: {
      handler(val) {
        console.log(this.taskName)
        this.showSnapshot = !(this.taskName === "");
        if (this.showSnapshot) {
          let baseUrl = "/file_sys/snapshots/chtdet_" +
            this.taskName + "/"
          this.url.baseUrl = baseUrl
          this.url.descUrl = baseUrl + ".desc"
          this.updateMaxNum()
        }
      },
      immediate: true
    }
  }
}
</script>

<style scoped>
.snapshot_board {
  border-radius: 2px;
  display: flex;
  flex-wrap: wrap;
}

#components-pagination-demo-mini .ant-pagination:not(:last-child) {
  margin-bottom: 24px;
}

</style>