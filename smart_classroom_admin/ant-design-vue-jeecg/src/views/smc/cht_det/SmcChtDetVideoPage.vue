<template>
  <a-card :bordered="false">
    <div v-if="testMode">
      <a-input v-model="taskId"></a-input>
      <a-button type="primary"
                icon="forward"
                shape="circle"
                @click="init_start"></a-button>
    </div>
    <a-row>
      <a-col :span="16">
        <a-card>
          <video autoplay controls width="100%" height=500 ref="videoElement"></video>
        </a-card>
      </a-col>
      <a-col :span="8">
        <smc-cht-det-snapshots :task-name="currentTaskName"
                               :task-obj="taskObj"></smc-cht-det-snapshots>
      </a-col>
    </a-row>
  </a-card>
</template>

<script>
import flvjs from 'flv.js'
import SmcChtDetSnapshots from "./modules/SmcChtDetSnapshots";
import {getAction} from '@api/manage'

export default {
  name: "CheatingDetection",
  components: {SmcChtDetSnapshots},
  data() {
    return {
      testMode: false,
      taskObj: {},
      videoSrc: {},
      isLive: true,
      isCatchFrame: false,
      taskId: "",
      flvPlayer: null,
      catchFrameTimer: null,
      currentTaskName: "",
      url: {
        queryChtDetById: "/smc/smcChtDet/queryById",
        queryVideoSrcById: "/smc/smcVideoSrc/queryById",
      }
    }
  },
  created() {
    this.taskId = this.$route.params.task_id || ""
    this.testMode = !!!this.taskId
    if (!this.testMode) {
      this.init_start()
    }
  },
  destroyed() {
    if (this.catchFrameTimer != null) {
      clearInterval(this.catchFrameTimer)
      this.catchFrameTimer = null
    }
    this.destroyVideo()
  },
  methods: {
    init_start() {
      const this_ = this
      const zg = /^(?![0-9]+$)$/;
      if (zg.test(this.taskId)) {
        this.$message.warning("任务id由数字组成")
        return;
      }
      getAction(this.url.queryChtDetById, {id: this.taskId}).then(async (resp) => {
        if (resp.success) {
          // console.log(resp.result)
          this_.taskObj = resp.result
          if (await this_.init_net_connections_info()) {
            this_.startUp()
          }
        } else {
          this.$message.warning("找不到监考任务" + this_.taskId)
        }
      })
    },
    async init_net_connections_info() {
      let videoSrcAccess = await getAction(this.url.queryVideoSrcById, {id: this.taskObj.videoSrcId})
      if (videoSrcAccess.success) {
        this.videoSrc = videoSrcAccess.result
      } else {
        return false
      }
      return true
    },
    startUp() {
      this.destroyVideo()
      this.createVideo()
      this.currentTaskName = this.taskId; // 启动snapshots
    },
    destroyVideo() {
      if (this.flvPlayer != null) {
        this.flvPlayer.pause()
        this.flvPlayer.unload();
        this.flvPlayer.detachMediaElement();
        this.flvPlayer.destroy()
        this.flvPlayer = null
      }
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

          // enableWorker: true,
          // enableStashBuffer: false,
          // stashInitialSize: 128,
          // autoCleanupSourceBuffer: true,
          url: '/rtmp/live?port=1935&app=demo&stream=chtdet_' + this.taskId //你的url地址
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
              const maxPlaybackRate = 1.5;// 自定义设置允许的最大播放速度
              let playbackRate = 1.0; // 播放速度
              if (diff > diffCritical) {
                videoElement.currentTime = end - 4;
                playbackRate = Math.max(1, Math.min(maxPlaybackRate, 16));
              } else if (diff > diffSpeedUp) {
                playbackRate = Math.max(1, Math.min(diff, maxPlaybackRate, 16))
              }
              videoElement.playbackRate = playbackRate;
            }
          }, 2000)
        }
      }
    }
  }
}
</script>

<style scoped>

</style>