<template>
    <div class="example">
        <a-button type="primary" @click="test">
            Get system info
        </a-button>
        <!-- <p>{{ uptime.toISOString().substring(11, 19) }}</p> -->
        <!--
        <div>
            <p>CPU load {{ cpuPercentage }}%</p>
            <p>Memory {{ memPercentage }}%</p>
            <p>Flash {{ flashPercentage }}%</p>
            <p v-for="info in sysinfo" :key="info[0]">{{ info[0] }} {{ info[1] }}</p>
        </div>
        -->

        <div v-if="wan.name">
            <p>Type {{ wan.name }}</p>
            <p>Ip address {{ wan.getIPv4Addrs().join(', ') }}</p>
        </div>

        <div v-if="lan.nameyy">
            <p>Type {{ lan.name }}</p>
            <p>Ip address {{ lan.getIPv4Addrs().join(', ') }}</p>
        </div>

        <!--
        <card title="System">
            <p>CPU load <a-progress :percent="cpuPercentage"/></p>
            <p>Memory <a-progress :percent="memPercentage"/></p>
            <p>Flash <a-progress :percent="flashPercentage"/></p>
            <p v-for="info in sysinfo" :key="info[0]">{{ info[0] }} {{ info[1] }}</p>
            <template slot="abc">abc</template>
            <template slot="abc">abc</template>
        </card>
        -->
        <card title="System" :data="testData"></card>
    </div>
</template>

<script>
import Card from './Card.vue'

export default {
  components: { Card },
  data () {
    return {
      // System info
      sysinfo: [],
      memPercentage: 100,
      flashPercentage: 100,

      // CPU time
      cpuPercentage: null,
      lastCPUTime: null,

      // WAN and LAN
      wan: [],
      lan: [],

      testData: [
        { name: 'Test', data: 'Data', progress: false, columns: 0 },
        { name: 'Test1', data: 'Data2', progress: false, columns: 0 },
        { name: 'Columns', columns: 2, columnTitles: ['Memory', 'Flash'], data: ['One', 'Two'], progress: [false, false] }
      ]
    }
  },
  timers: {
    // update: { time: 2000, autostart: true, immediate: true, repeat: true },
    // getCpuTime: { time: 1000, autostart: true, immediate: true, repeat: true }
  },
  methods: {
    test () {
      /*
      this.$rpc.call('system', 'syslog', { limit: 10 }).then(({ log }) => {
        console.log(log)
      })
      */

      this.$log.get({ table: 'NETWORK', limit: 5 }).then((r) => {
        console.log(r)
      })

      this.$log.get({ table: 'SYSTEM', limit: 5 }).then((r) => {
        console.log(r)
      })

      // this.getWan()
      // console.log(this.wan)

    // this.getCpuTime()
    // this.getSysInfo()
    },
    update () {
      this.getWan()
      this.getLan()
      this.getSysInfo()
      console.log(this.wan)
    },
    getWan () {
      this.$network.load().then(() => {
        const iface = this.$network.getInterface('wan')
        if (!iface) {
          this.waninfo = []
          this.wanIsUp = false
          return
        }

        this.waninfo = [
          [this.$t('home.IP Address'), iface.getIPv4Addrs()],
          [this.$t('home.Gateway'), iface.getIPv4Gateway()],
          ['DNS', iface.getDNSAddrs().join(', ')]
        ]
        this.wanIsUp = iface.isUp()
        this.wan = iface
      })
    },
    getLan () {
      this.$network.load().then(() => {
        const iface = this.$network.getInterface('lan')
        if (!iface) {
          this.waninfo = []
          this.wanIsUp = false
          return
        }

        this.waninfo = [
          [this.$t('home.IP Address'), iface.getIPv4Addrs()],
          [this.$t('home.Gateway'), iface.getIPv4Gateway()],
          ['DNS', iface.getDNSAddrs().join(', ')]
        ]
        this.wanIsUp = iface.isUp()
        this.lan = iface
      })
    },
    getCpuTime () {
      this.$rpc.call('system', 'cpu_time').then(times => {
        if (!this.lastCPUTime) {
          this.cpuPercentage = 0
          this.lastCPUTime = times
          return
        }

        // CPU load
        const idle1 = this.lastCPUTime[3]
        const idle2 = times[3]
        let total1 = 0
        let total2 = 0

        this.lastCPUTime.forEach(t => {
          total1 += t
        })

        times.forEach(t => {
          total2 += t
        })

        this.cpuPercentage = Math.floor(((total2 - total1 - (idle2 - idle1)) / (total2 - total1)) * 100)
      })
    },
    getSysInfo () {
      this.$system.getInfo().then(({ disk, release, localtime, uptime, memory }) => {
        let localDate = new Date(localtime * 1000)
        localDate = localDate.toLocaleDateString('lt-LT') + ' ' + localDate.toLocaleTimeString('lt-LT')

        this.sysinfo = [
          ['ROUTER UPTIME', '%t'.format(uptime)],
          ['LOCAL DEVICE TIME', localDate],
          ['FIRMWARE VERSION', release.revision]
        ]

        this.memPercentage = Math.floor(((memory.total - memory.free) / memory.total) * 100)
        this.flashPercentage = Math.floor((disk.root.used / disk.root.total) * 100)
      // console.log((disk.root.used / disk.root.total) * 100)
      })
    }
  }
}
</script>

<style></style>
