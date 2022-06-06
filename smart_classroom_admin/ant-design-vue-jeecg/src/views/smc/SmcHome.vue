<template>
  <div>
    <a-row :gutter="24">
      <a-col :sm="24" :md="12" :xl="6" :style="{ marginBottom: '24px' }">
        <chart-card :loading="loading" title="注册考生人数" :total="students.total + '人'">
          <a-tooltip title="系统中注册的考生人数" slot="action">
            <a-icon type="info-circle-o"/>
          </a-tooltip>
          <template slot="footer">注册考生<span> {{ students.total }} 人</span></template>
        </chart-card>
      </a-col>
      <a-col :sm="24" :md="12" :xl="6" :style="{ marginBottom: '24px' }">
        <chart-card :loading="loading" title="考场信息"
                    :total="classroom.total + '间'">
          <a-tooltip title="考场相关统计信息" slot="action">
            <a-icon type="info-circle-o"/>
          </a-tooltip>
          <template slot="footer">摄像头数量<span> {{ classroom.videos }}</span></template>
        </chart-card>
      </a-col>
      <a-col :sm="24" :md="12" :xl="6" :style="{ marginBottom: '24px' }">
        <chart-card :loading="loading" title="考试次数" :total="olMtrs.total + '场'">
          <a-tooltip title="考试相关统计信息" slot="action">
            <a-icon type="info-circle-o"/>
          </a-tooltip>
          <div>
            <mini-progress color="rgb(19, 194, 194)"
                           :target="100"
                           :percentage="olMtrs.total===0 ? 0 : olMtrs.finish/olMtrs.total*100"
                           :height="8"/>
          </div>
          <template slot="footer">
            <span slot="term">进行中</span>
            {{ olMtrs.total - olMtrs.finish }}
            <span slot="term">已结束</span>
            {{ olMtrs.finish }}
          </template>
        </chart-card>
      </a-col>
      <a-col :sm="24" :md="12" :xl="6" :style="{ marginBottom: '24px' }">
        <chart-card :loading="loading" title="AI服务器" :total="aiServer.total + '台'">
          <a-tooltip title="算法服务器相关统计信息" slot="action">
            <a-icon type="info-circle-o"/>
          </a-tooltip>
          <div>
            <mini-progress color="rgb(19, 194, 194)"
                           :target="100"
                           :percentage="aiServer.total===0 ? 0 : aiServer.available/aiServer.total*100"
                           :height="8"/>
          </div>
          <template slot="footer">
            <span slot="term">可用 </span>
            {{ aiServer.available }}
            <span slot="term">待重连 </span>
            {{ aiServer.total - aiServer.available }}
          </template>
        </chart-card>
      </a-col>
    </a-row>

    <a-row>
      <a-col :span="24">
        <a-card :loading="loading" :bordered="false" title="最近一周访问量统计" :style="{ marginTop: '24px' }">
          <a-row>
            <a-col :span="6">
              <head-info title="今日IP" :content="loginfo.todayIp"></head-info>
            </a-col>
            <a-col :span="2">
              <a-spin class='circle-cust'>
                <a-icon slot="indicator" type="environment" style="font-size: 24px"/>
              </a-spin>
            </a-col>
            <a-col :span="6">
              <head-info title="今日访问" :content="loginfo.todayVisitCount"></head-info>
            </a-col>
            <a-col :span="2">
              <a-spin class='circle-cust'>
                <a-icon slot="indicator" type="team" style="font-size: 24px"/>
              </a-spin>
            </a-col>
            <a-col :span="6">
              <head-info title="总访问量" :content="loginfo.totalVisitCount"></head-info>
            </a-col>
            <a-col :span="2">
              <a-spin class='circle-cust'>
                <a-icon slot="indicator" type="rise" style="font-size: 24px"/>
              </a-spin>
            </a-col>
          </a-row>
          <line-chart-multid :fields="visitFields" :dataSource="visitInfo"></line-chart-multid>
        </a-card>
      </a-col>
    </a-row>
  </div>
</template>

<script>
import ChartCard from '@/components/ChartCard'
import ACol from "ant-design-vue/es/grid/Col"
import ATooltip from "ant-design-vue/es/tooltip/Tooltip"
import MiniArea from '@/components/chart/MiniArea'
import MiniBar from '@/components/chart/MiniBar'
import MiniProgress from '@/components/chart/MiniProgress'
import RankList from '@/components/chart/RankList'
import Bar from '@/components/chart/Bar'
import LineChartMultid from '@/components/chart/LineChartMultid'
import HeadInfo from '@/components/tools/HeadInfo.vue'

import Trend from '@/components/Trend'
import {getLoginfo, getVisitInfo} from '@/api/api'
import {getAction} from "@api/manage";

export default {
  name: "SmcHome",
  components: {
    ATooltip,
    ACol,
    ChartCard,
    MiniArea,
    MiniBar,
    MiniProgress,
    RankList,
    Bar,
    Trend,
    LineChartMultid,
    HeadInfo
  },
  data() {
    return {
      loading: true,
      center: null,
      loginfo: {},
      visitFields: ['ip', 'visit'],
      visitInfo: [],
      indicator: <a-icon type="loading" style="font-size: 24px" spin/>,
      url: {
        students: "/smc/home/students",
        aiServer: "/smc/home/aiServer",
        classroom: "/smc/home/classroom",
        olMtrs: "/smc/home/olMtrs", //在线监考任务
      },
      classroom: {
        total: 0,
        videos: 0
      },
      students: {
        total: 0
      },
      aiServer: {
        total: 0,
        available: 0
      },
      olMtrs: {
        total: 0,
        finish: 0
      }
    }
  },
  created() {
    setTimeout(() => {
      this.loading = !this.loading
    }, 1000)
    this.initLogInfo()
    this.initInfo()
  },
  methods: {
    initInfo() {
      let that = this
      getAction(this.url.aiServer).then(res => {
        if (res.success) {
          that.aiServer = res.result
        } else {
          this.$message.warning("算法服务器数据查询错误")
        }
      })
      getAction(this.url.students).then(res => {
        if (res.success) {
          that.students = res.result
        } else {
          this.$message.warning("学生数据查询错误")
        }
      })
      getAction(this.url.classroom).then(res => {
        if (res.success) {
          that.classroom = res.result
        } else {
          this.$message.warning("考场数据查询错误")
        }
      })
      getAction(this.url.olMtrs).then(res => {
        if (res.success) {
          that.olMtrs = res.result
        } else {
          this.$message.warning("考试数据查询错误")
        }
      })
    },
    initLogInfo() {
      getLoginfo(null).then((res) => {
        if (res.success) {
          Object.keys(res.result).forEach(key => {
            res.result[key] = res.result[key] + ""
          })
          this.loginfo = res.result;
        }
      })
      getVisitInfo().then(res => {
        if (res.success) {
          this.visitInfo = res.result;
        }
      })
    },
  }
}
</script>

<style lang="less" scoped>
.circle-cust {
  position: relative;
  top: 28px;
  left: -100%;
}

.extra-wrapper {
  line-height: 55px;
  padding-right: 24px;

  .extra-item {
    display: inline-block;
    margin-right: 24px;

    a {
      margin-left: 24px;
    }
  }
}

/* 首页访问量统计 */
.head-info {
  position: relative;
  text-align: left;
  padding: 0 32px 0 0;
  min-width: 125px;

  &.center {
    text-align: center;
    padding: 0 32px;
  }

  span {
    color: rgba(0, 0, 0, .45);
    display: inline-block;
    font-size: .95rem;
    line-height: 42px;
    margin-bottom: 4px;
  }

  p {
    line-height: 42px;
    margin: 0;

    a {
      font-weight: 600;
      font-size: 1rem;
    }
  }
}
</style>