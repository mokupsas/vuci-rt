<template>
    <div class="example">
        <a-button type="primary" @click="test">
            Update cards
        </a-button>

        <card
          v-for="(card, key) in cardsData" :key="key"
          :title="card.title"
          :data="card.rows"
        ></card>

    </div>
</template>

<script>
import Card from './Card.vue'

const addRowProperties = obj => {
  obj.title = obj[0]
  obj.value = obj[1]
  obj.type = obj[2]
}

export default {
  components: { Card },
  data () {
    return {
      cardsData: [
        /*
        {
          title: 'SYSTEM',
          rows: [
            ['CPU load', 50],
            ['CPU load1', 50]
          ]
        }
        */
      ],
      // System data
      sysInfo: [
        { name: 'CPU load', data: 0, type: 'progress-bar' },
        { name: 'Router uptime', data: null },
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
    update: { time: 2000, autostart: true, immediate: true, repeat: true }
    // getCpuTime: { time: 1000, autostart: true, immediate: true, repeat: true }
  },
  methods: {
    async test () {
      const sysRows = await this.getSysInfo()
      const card = this.createCard('SYSTEM', sysRows)
      const changedCard = this.getCardChanges(this.cardsData, card)

      if (changedCard !== false) {
        const index = this.cardsData.findIndex(item => item.title === card.title)
        this.cardsData[index].rows = card.rows
        console.log('card has changed')
      } else {
        console.log('card didnt change')
      }
    },
    /**
     * Creates card object
     * @param {string} title card title/heading
     * @param {array} rows array of rows
     * @return {object} card object
     */
    createCard (title, rows) {
      return {
        title: title,
        rows: rows
      }
    },
    /**
     * Gets an array of cards with the same title but changed details
     * @param {array} cardsData array of cards
     * @param {object} card card object
     * @returns {array|bool} array of cards that have changed | false on empty array
     */
    getCardChanges (cardsData, card) {
      /*
      if (cardsData.length === 0) { return true }
      const val = cardsData.filter((item) => {
        if (item.title === card.title) {
          console.log('a')
          console.log(item.rows)
          console.log(card.rows)
          return JSON.stringify(item.rows) !== JSON.stringify(card.rows)
        }
        return true
      })
      console.log(val)
      */

      const res = cardsData.filter(({ title, rows }) => title === card.title && JSON.stringify(rows) !== JSON.stringify(card.rows))
      if (res.length === 0) return false
      return res
    },
    /**
     *  Inserts card into array and returns it's index
     * @param {*} cardsData array of cards
     * @param {*} card card to insert
     * @return {int} index
     */
    insertCard (cardsData, card) {
      cardsData.push(card)
      return cardsData.length - 1
    },
    /**
     * Updates existing card with new rows
     * @param {array} cards array of cards
     * @param {int} key card index
     * @param {array} rows array of rows
     */
    updateCard (cards, key, rows) {
      this.addPropsAllRows(rows)
      cards[key].rows = rows
    },
    /**
     * Add properties to each row in array and returns be reference
     * @param {array} rows
     */
    addPropsAllRows (rows) {
      // Adding properties to each row
      rows.forEach(row => {
        addRowProperties(row)
      })
    },
    /**
     *Updates every card in cards array. Called by timing
     */
    async update () {
      /*
      const sysRows = await this.getSysInfo()
      const card = this.getCardsData('SYSTEM', sysRows)
      this.insertCard(this.cardsData, card)
      */
    },
    /**
     * Gets system data and retuns array of rows
     * @return {array} array of rows
     */
    async getSysInfo () {
      const data = await this.$system.getInfo().then((response) => {
        return response
      })
      if (!data) { return {} }

      const cpuLoad = await this.getCpuLoad()
      const memUsage = Math.floor(((data.memory.total - data.memory.free) / data.memory.total) * 100)
      const flashUsage = Math.floor((data.disk.root.used / data.disk.root.total) * 100)

      // Formating rows for card
      const rows = [
        ['CPU LOAD', cpuLoad, 'progress-bar'],
        ['ROUTER UPTIME', '%t'.format(data.uptime)],
        ['LOCAL DEVICE TIME', this.toDate(data.localtime)],
        ['MEMORY USAGE', memUsage, 'progress-bar'],
        ['FLASH USAGE', flashUsage, 'progress-bar'],
        ['FIRMWARE VERSION', data.release.revision]
      ]

      this.addPropsAllRows(rows)

      return rows
    },
    async getCpuLoad () {
      const load = await this.$rpc.call('system', 'cpu_time').then(times => {
        // CPU load
        const idle = times[3]
        let total = 0

        times.forEach(t => {
          total += t
        })

        return Math.floor(((total - idle) / total) * 100) // cpu load
      })

      return load
    },
    async getInterfacesInfo () {
      return await this.$network.load().then(() => {
        const ifaces = this.$network.getInterfaces()

        // Formating rows for card
        const rows = []

        ifaces.forEach(iface => {
          rows.push([
            iface.name,
            iface.getIPv4Addrs().join(' ')
          ])
        })

        // Adding properties to each row
        rows.forEach(row => {
          addRowProperties(row)
        })

        return rows
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
  },
  async created () {
    const sysRows = await this.getSysInfo()
    const card = this.createCard('SYSTEM', sysRows)
    this.insertCard(this.cardsData, card)
  }
}
</script>

<style></style>
