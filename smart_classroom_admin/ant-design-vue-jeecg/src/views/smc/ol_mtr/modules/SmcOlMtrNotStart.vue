<template>
  <div>
    <h1 class="lastTime0">离开始时间剩余</h1>
    <h1 class="lastTime"><label>{{ lastTime }}</label></h1>
    <a-row>
      <a-col :span="24">
        <a-button v-if="lastTime==='00:00:00'"
                  style="display:block;margin:0 auto"
                  size="large" type="primary"
                  @click="nextStep">进行考生身份认证
        </a-button>
      </a-col>
    </a-row>
  </div>
</template>

<script>
import moment from "moment";

export default {
  name: "SmcOlMtrNotStart",
  props: ["taskObj"],
  data() {
    return {
      lastTime: "00:00:01",
      lastTimeDuration: null,
      timeTimer: null
    }
  },
  created() {
  },
  methods: {
    nextStep() {
      this.$emit("nextStep");
    }
  },
  watch: {
    taskObj: {
      handler(val) {
        if (this.taskObj) {
          if (this.timeTimer == null) {
            let that = this
            that.lastTime = "00:00:00"
            let startTime = moment(this.taskObj.startTime, "YYYY-MM-DD HH:mm:ss")
            this.lastTimeDuration = moment.duration(startTime.diff(moment()))
            let updateFunc = function () {
              if (that.lastTimeDuration && that.lastTimeDuration.as("seconds") > 0) {
                that.lastTimeDuration.subtract(1, "seconds")
                let hours = Math.max(Math.floor(that.lastTimeDuration.asHours()), 0).toString().padStart(2, "0")
                let mins = that.lastTimeDuration.minutes().toString().padStart(2, "0")
                let seconds = that.lastTimeDuration.seconds().toString().padStart(2, "0")
                that.lastTime = hours + ":" + mins + ":" + seconds
              } else {
                clearInterval(that.timeTimer)
                that.timeTimer = null
              }
            }
            updateFunc()
            this.timeTimer = setInterval(updateFunc, 1000)
          }
        }
      },
      immediate: true
    }
  }
}
</script>

<style scoped>
.lastTime0 {
  text-align: center;
}

.lastTime {
  text-align: center;
  font-size: 5vw;
}
</style>