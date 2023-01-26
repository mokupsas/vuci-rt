<template>
    <div class="example">
        <a-button type="primary" @click="test">
            Get system info
        </a-button>

        <a-button type="primary" @click="openDrawer">
            Drawer
        </a-button>

        <card title="SYSTEM" :data="sysInfo"></card>
        <card title="LAN" :data="lan"></card>
        <card title="WAN" :data="wan"></card>
        <card title="RECENT NETWORK EVENTS" :data="netEvents"></card>
        <card title="RECENT SYSTEM EVENTS" :data="sysEvents"></card>

    <a-drawer
      title="Basic Drawer"
      placement="right"
      :closable="false"
      :visible="drawer"
      @close="openDrawer">
      <p draggable>Some contents1...</p>
      <p draggable>Some contents2...</p>
      <p draggable>Some contents3...</p>
    </a-drawer>

    </div>
</template>

<script>
import Card from './Card.vue'

export default {
  components: { Card },
  data () {
    return {
      drawer: false,

      // System data
      sysInfo: [
        { name: 'CPU load', data: 0, progress: true },
        { name: 'Router uptime', data: null, progress: false },
        { name: 'Local device time', data: null, progress: false },
        { name: 'Memory usage', data: 0, progress: true },
        { name: 'Flash usage', data: 0, progress: true },
        { name: 'Firmware version', data: null, progress: false }
      ],

      wan: [
        { name: 'Type', data: null, progress: false },
        { name: 'IP address', data: null, progress: false }
      ],
      lan: [
        { name: 'Type', data: null, progress: false },
        { name: 'IP address', data: null, progress: false }
      ],
      netEvents: [],
      sysEvents: []
    }
  },
  timers: {
    update: { time: 2000, autostart: true, immediate: true, repeat: true },
    getCpuTime: { time: 1000, autostart: true, immediate: true, repeat: true }
  },
  methods: {
    test () {
      // this.getEvents(this.netEvents, 'NETWORK', 5)
      // console.log(this.netEvents)
      // this.getEvents(this.netEvents, 'NETWORK', 5)
      // console.log(this.netEvents)
    },
    openDrawer () {
      this.drawer = !this.drawer
    },
    update () {
      this.getSysInfo()
      this.getInterface(this.lan, 'lan')
      this.getInterface(this.wan, 'wan')
      this.getEvents(this.netEvents, 'NETWORK', 5)
      this.getEvents(this.sysEvents, 'SYSTEM', 5)
    },
    getInterface (val, inter) {
      this.$network.load().then(() => {
        const iface = this.$network.getInterface(inter)
        if (!iface) return

        val[0].data = `Wireless (${iface.getDevice().name})`
        val[1].data = iface.getIPv4Addrs().join(' ')
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

        this.sysInfo[0].data = Math.floor(((total2 - total1 - (idle2 - idle1)) / (total2 - total1)) * 100) // cpu load
      })
    },
    getSysInfo () {
      this.$system.getInfo().then(({ disk, release, localtime, uptime, memory }) => {
        let localDate = new Date(localtime * 1000)
        localDate = localDate.toLocaleDateString('lt-LT') + ' ' + localDate.toLocaleTimeString('lt-LT')

        // Setting data
        this.sysInfo[1].data = '%t'.format(uptime) // uptime
        this.sysInfo[2].data = localDate // local device time
        this.sysInfo[3].data = Math.floor(((memory.total - memory.free) / memory.total) * 100) // memory usage
        this.sysInfo[4].data = Math.floor((disk.root.used / disk.root.total) * 100) // flash usage
        this.sysInfo[5].data = release.revision // firmware
      })
    },
    getEvents (ref, type, lim) {
      const arr = []
      this.$log.get({ table: type, limit: lim }).then((r) => {
        r.log.forEach((event) => {
          const obj = { name: this.toDate(event.TIME), data: event.TEXT, progress: false }
          arr.push(obj)
        })
        this.copyArray(ref, arr)
      })
    },
    // Timestamp to date
    toDate (timestamp) {
      let localDate = new Date(timestamp * 1000)
      localDate = localDate.toLocaleDateString('lt-LT') + ' ' + localDate.toLocaleTimeString('lt-LT')
      return localDate
    },
    copyArray (to, from) {
      // Emptying old data
      to.splice(0, to.length) // without splice, array isn't server by reference

      from.forEach(item => {
        to.push(item)
      })
    }
  }
}
</script>

<style></style>
