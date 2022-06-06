<template>
  <div>
    <a-row>
      <a-col :span="6">
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
      <a-col :span="18">
        <a-row>
          <a-col :span="6">
            <a-button @click="stopTask" type="danger">
              结束认证
            </a-button>
          </a-col>
        </a-row>
        <a-row>
          <a-card>
            <video autoplay controls width="100%" height=475 ref="videoElement"></video>
          </a-card>
        </a-row>
      </a-col>
    </a-row>
  </div>
</template>

<script>
import {getAction} from "@api/manage";
import axios from "axios";
import moment from "dayjs";
import flvjs from "flv.js";

export default {
  name: "SmcOlMtrDynAtt",
  props: ["taskObj"],
  data() {
    return {
      taskId: "",
      showSnapshot: false,
      taskInfo: {},
      isLive: true,
      isCatchFrame: false,
      url: {
        baseUrl: "",
        descUrl: "",
        queryDynAttById: "/smc/smcDynAtt/queryById",
        querySmcClassMemByMainId: "/smc/smcClass/querySmcClassMemByMainId",
        alive: "/smc/smcDynAttExtra/alive",
        stop: "/smc/smcDynAttExtra/stop"
      },
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
      updateTimer: null,
      aliveTimer: null,
      catchFrameTimer: null,
      descTimeStamp: 0,
    }
  },
  methods: {
    destroyVideo() {
      if (this.flvPlayer != null) {
        this.flvPlayer.pause()
        this.flvPlayer.unload();
        this.flvPlayer.detachMediaElement();
        this.flvPlayer.destroy()
        this.flvPlayer = null
      }
    },
    async updateList() {
      //获取班级数据
      this.taskInfo = (await getAction(this.url.queryDynAttById,
        {id: this.taskId})).result
      if (!!!this.taskInfo) {
        return;
      }
      this.classMems = (await getAction(this.url.querySmcClassMemByMainId,
        {id: this.taskInfo.classId})).result
      if (!!!this.classMems) {
        return
      }
      //轮询更新表格
      let that = this
      if (!!this.updateTimer) {
        clearInterval(this.updateTimer)
      }
      let updateFunc = function () {
        axios.get(that.url.descUrl).then(function (resp) {
          resp = resp.data
          if (resp.timestamp !== that.descTimeStamp) {
            that.setList(resp)
            that.descTimeStamp = resp.timestamp
          }
        })
      }
      updateFunc()
      this.updateTimer = setInterval(updateFunc, 1000)
      //延长任务计时器
      if (!!this.aliveTimer) {
        clearInterval(this.aliveTimer)
      }
      let currentTime = moment().format("YYYY-MM-DD HH:mm:ss")

      if (this.taskInfo.endTime > currentTime) {
        let aliveFunc = function () {
          getAction(that.url.alive, {id: that.taskId}).then(function (resp) {
            if (!resp.success) {
              that.stopTask()
            }
          })
        }
        aliveFunc()
        this.aliveTimer = setInterval(aliveFunc, 50000)
      }
      //打开视频
      this.createVideo()
    },
    stopTask() {
      if (!!this.aliveTimer) {
        clearInterval(this.aliveTimer)
      }
      this.$emit("nextStep");
    },
    setList(desc) {
      // console.log(desc)
      let att_list = []
      let abs_list = []
      let attendace = desc.attendance
      let that = this
      this.classMems.forEach(function (f, i) {
        // classId: "1500481204264714242"
        // createBy: "admin"
        // createTime: "2022-03-11 15:41:47"
        // faceRegId: "1502180677869203458"
        // id: "1502188018962395138"
        // memName: "郭学剑"
        // sysOrgCode: "A01"
        // updateBy: null
        // updateTime: null
        // userId: ""
        if (attendace[f.id]) {
          let matchInfo = attendace[f.id]
          // confidence: 0.7229406237602234
          // idx: 2
          // timestamp: 1647178558524
          att_list.push({
            name: f.memName,
            img: that.url.baseUrl + matchInfo.idx + ".jpg?t=" + matchInfo.timestamp.toString(),
            confidence: (matchInfo.confidence * 100).toFixed(2) + "%"
          })
        } else {
          abs_list.push({name: f.memName})
        }
      })
      this.att_list = att_list
      this.abs_list = abs_list
    },
    createVideo() {
      let that = this
      if (flvjs.isSupported()) {
        const videoElement = this.$refs.videoElement;
        this.destroyVideo()
        this.flvPlayer = flvjs.createPlayer({
          type: 'flv',
          isLive: this.isLive,
          hasAudio: false,
          hasVideo: true,
          url: '/rtmp/live?port=1935&app=demo&stream=dynatt_' + this.taskId //你的url地址
        }, {
          isLive: this.isLive,
          enableStashBuffer: false,
          autoCleanupSourceBuffer: true
        });
        this.flvPlayer.attachMediaElement(videoElement);
        this.flvPlayer.load();
        this.flvPlayer.play();

        if (this.catchFrameTimer != null) {
          clearInterval(this.catchFrameTimer)
          this.catchFrameTimer = null
        }
        if (this.isLive && this.isCatchFrame) {
          this.catchFrameTimer = setInterval(function () {
            let videoElement = that.$refs.videoElement
            let buf = videoElement.buffered
            if (buf.length > 0) {
              const end = buf.end(0);  // 视频结尾时间
              const current = videoElement.currentTime;  //  视频当前时间
              const diff = end - current;// 相差时间
              const diffCritical = 4; // 这里设定了超过4秒以上就进行跳转
              const diffSpeedUp = 1; // 这里设置了超过1秒以上则进行视频加速播放
              const maxPlaybackRate = 4;// 自定义设置允许的最大播放速度
              let playbackRate = 1.0; // 播放速度
              if (diff > diffCritical) {
                videoElement.currentTime = end - 1.5;
                playbackRate = Math.max(1, Math.min(diffCritical, 16));
              } else if (diff > diffSpeedUp) {
                playbackRate = Math.max(1, Math.min(diff, maxPlaybackRate, 16))
              }
              videoElement.playbackRate = playbackRate;
            }
          }, 2000)
        }
      }
    }
  },
  created() {
    if (this.taskObj) {
      this.taskId = this.taskObj.dynAttId
      this.showSnapshot = this.taskId && !(this.taskId === "");
      if (this.showSnapshot) {
        let baseUrl = "/file_sys/snapshots/dynatt_" +
          this.taskId + "/"
        this.url.baseUrl = baseUrl
        this.url.descUrl = baseUrl + ".desc"
        this.updateList()
      }
    }
  },
  destroyed() {
    clearInterval(this.updateTimer)
    clearInterval(this.catchFrameTimer)
    clearInterval(this.aliveTimer)
    this.destroyVideo()
  }
}
</script>

<style scoped>

</style>