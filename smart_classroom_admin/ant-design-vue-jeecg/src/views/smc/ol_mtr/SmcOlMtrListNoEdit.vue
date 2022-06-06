<template>
  <a-card :bordered="false">
    <!-- 查询区域 -->
    <div class="table-page-search-wrapper">
      <a-form layout="inline" @keyup.enter.native="searchQuery">
        <a-row :gutter="24">
        </a-row>
      </a-form>
    </div>
    <!-- 查询区域-END -->

    <!-- 操作按钮区域 -->
    <div class="table-operator">
      <a-button type="primary" icon="download" @click="handleExportXls('智慧教室在线监考')">导出</a-button>
      <!-- 高级查询区域 -->
      <j-super-query :fieldList="superFieldList" ref="superQueryModal"
                     @handleSuperQuery="handleSuperQuery"></j-super-query>
      <a-dropdown v-if="selectedRowKeys.length > 0">
        <a-button style="margin-left: 8px"> 批量操作
          <a-icon type="down"/>
        </a-button>
      </a-dropdown>
    </div>

    <!-- table区域-begin -->
    <div>
      <div class="ant-alert ant-alert-info" style="margin-bottom: 16px;">
        <i class="anticon anticon-info-circle ant-alert-icon"></i> 已选择 <a
        style="font-weight: 600">{{ selectedRowKeys.length }}</a>项
        <a style="margin-left: 24px" @click="onClearSelected">清空</a>
      </div>

      <a-table
        ref="table"
        size="middle"
        :scroll="{x:true}"
        bordered
        rowKey="id"
        :columns="columns"
        :dataSource="dataSource"
        :pagination="ipagination"
        :loading="loading"
        :rowSelection="{selectedRowKeys: selectedRowKeys, onChange: onSelectChange}"
        class="j-table-force-nowrap"
        @change="handleTableChange">

        <template slot="htmlSlot" slot-scope="text">
          <div v-html="text"></div>
        </template>
        <template slot="imgSlot" slot-scope="text">
          <span v-if="!text" style="font-size: 12px;font-style: italic;">无图片</span>
          <img v-else :src="getImgView(text)" height="25px" alt=""
               style="max-width:80px;font-size: 12px;font-style: italic;"/>
        </template>
        <template slot="fileSlot" slot-scope="text">
          <span v-if="!text" style="font-size: 12px;font-style: italic;">无文件</span>
          <a-button
            v-else
            :ghost="true"
            type="primary"
            icon="download"
            size="small"
            @click="downloadFile(text)">
            下载
          </a-button>
        </template>

        <span slot="action" slot-scope="text, record">
          <a @click="handleTask(record)">进入考场</a>

          <a-divider type="vertical"/>
          <a-dropdown>
            <a class="ant-dropdown-link">更多 <a-icon type="down"/></a>
            <a-menu slot="overlay">
              <a-menu-item>
                <a @click="handleDetail(record)">详情</a>
              </a-menu-item>
            </a-menu>
          </a-dropdown>
        </span>

      </a-table>
    </div>

    <smc-ol-mtr-modal ref="modalForm" @ok="modalFormOk"></smc-ol-mtr-modal>
  </a-card>
</template>

<script>

import '@/assets/less/TableExpand.less'
import {mixinDevice} from '@/utils/mixin'
import {JeecgListMixin} from '@/mixins/JeecgListMixin'
import SmcOlMtrModal from './modules/SmcOlMtrModal'

export default {
  name: 'SmcOlMtrListNoEdit',
  mixins: [JeecgListMixin, mixinDevice],
  components: {
    SmcOlMtrModal
  },
  data() {
    return {
      description: '智慧教室在线监考管理页面',
      // 表头
      columns: [
        {
          title: '#',
          dataIndex: '',
          key: 'rowIndex',
          width: 60,
          align: "center",
          customRender: function (t, r, index) {
            return parseInt(index) + 1;
          }
        },
        {
          title: '任务名称',
          align: "center",
          dataIndex: 'taskName'
        },
        {
          title: '开考时间',
          align: "center",
          dataIndex: 'startTime'
        },
        {
          title: '结束时间',
          align: "center",
          dataIndex: 'endTime'
        },
        {
          title: '关联考场',
          align: "center",
          dataIndex: 'classId_dictText'
        },
        {
          title: '考场监控',
          align: "center",
          dataIndex: 'videoSrcId_dictText'
        },
        {
          title: '任务状态',
          align: "center",
          dataIndex: 'taskStatus_dictText'
        },
        {
          title: '其他信息',
          align: "center",
          dataIndex: 'other'
        },
        {
          title: '操作',
          dataIndex: 'action',
          align: "center",
          fixed: "right",
          width: 147,
          scopedSlots: {customRender: 'action'}
        }
      ],
      url: {
        list: "/smc/smcOlMtr/list",
        delete: "/smc/smcOlMtr/delete",
        deleteBatch: "/smc/smcOlMtr/deleteBatch",
        exportXlsUrl: "/smc/smcOlMtr/exportXls",
        importExcelUrl: "smc/smcOlMtr/importExcel",

      },
      dictOptions: {},
      superFieldList: [],
    }
  },
  created() {
    this.getSuperFieldList();
  },
  computed: {
    importExcelUrl: function () {
      return `${window._CONFIG['domianURL']}/${this.url.importExcelUrl}`;
    },
  },
  methods: {
    handleTask(record) {
      this.$router.push({path: "/ol_mtr/task/" + record.id})
    },
    initDictConfig() {
    },
    getSuperFieldList() {
      let fieldList = [];
      fieldList.push({type: 'string', value: 'taskName', text: '任务名称', dictCode: ''})
      fieldList.push({type: 'datetime', value: 'startTime', text: '开考时间'})
      fieldList.push({type: 'datetime', value: 'endTime', text: '考试结束时间'})
      fieldList.push({
        type: 'sel_search',
        value: 'classId',
        text: '关联考场',
        dictTable: 'smc_class',
        dictText: 'class_name',
        dictCode: 'id'
      })
      fieldList.push({
        type: 'sel_search',
        value: 'videoSrcId',
        text: '考场监控',
        dictTable: 'smc_video_src',
        dictText: 'video_name',
        dictCode: 'id'
      })
      fieldList.push({type: 'int', value: 'taskStatus', text: '任务状态', dictCode: 'ol_mtr_status'})
      fieldList.push({type: 'Text', value: 'other', text: '其他信息', dictCode: ''})
      this.superFieldList = fieldList
    }
  }
}
</script>
<style scoped>
@import '~@assets/less/common.less';
</style>