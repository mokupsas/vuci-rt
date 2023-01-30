<template>
    <div class="example">
        <div class="cards-wrapper">
          <draggable v-model="cardsData">
            <card
              v-for="(card, key) in cardsData" :key="key"
              :title="card.title"
              :data="card.rows"
              draggable
            ></card>
          </draggable>
        </div>
    </div>
</template>

<script>
import draggable from 'vuedraggable'
import Card from './Card.vue'

const addRowProperties = obj => {
  obj.title = obj[0]
  obj.value = obj[1]
  obj.type = obj[2]
}

export default {
  components: { Card, draggable },
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
      ]
    }
  },
  timers: {
    update: { time: 1000, autostart: true, immediate: false, repeat: true }
  },
  methods: {
    async test () {
      const cards = await this.getCardsData()
      const changedCards = this.getCardsChanges(cards, this.cardsData)

      if (changedCards) {
        changedCards.forEach(card => {
          const index = this.cardsData.findIndex(item => item.title === card.title)
          this.cardsData[index].rows = card.rows
        })
      }
    },
    /**
     * Gets an array of cards
     */
    async getCardsData () {
      const cards = []

      // System card
      const sysCard = await this.getSysCard()
      cards.push(sysCard)

      // Interface cards
      const intCards = await this.getInterfacesCards()
      intCards.forEach(card => cards.push(card))

      // Network events card
      const netEvCard = await this.getEventsCard('NETWORK', 5)
      cards.push(netEvCard)

      // System events card
      const sysEvCard = await this.getEventsCard('SYSTEM', 5)
      cards.push(sysEvCard)

      return cards
    },
    /**
     * Gets an array of cards which data has changed
     * @param {*} card1 array of cards
     * @param {*} card2 array of cards to compare against
     * @return {array|bool} array of cards | false if no cards found
     */
    getCardsChanges (card1, card2) {
      const res = card1.filter(object1 => {
        return card2.some(object2 => {
          return object1.title === object2.title && JSON.stringify(object1.rows) !== JSON.stringify(object2.rows)
        })
      })
      if (res.length === 0) return false
      return res
    },
    /**
     * Adds properties to each row in array and returns be reference
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
      const cards = await this.getCardsData()
      const changedCards = this.getCardsChanges(cards, this.cardsData)

      if (changedCards) {
        changedCards.forEach(card => {
          const index = this.cardsData.findIndex(item => item.title === card.title)
          this.cardsData[index].rows = card.rows
        })
      }
    },
    /**
     * Gets system data and retuns array of rows
     * @return {array} array of rows
     */
    async getSysCard () {
      const data = await this.$system.getInfo().then((response) => {
        return response
      })
      if (!data) { return {} }

      const cpuLoad = await this.getCpuLoad()
      const memUsage = Math.floor(((data.memory.total - data.memory.free) / data.memory.total) * 100)
      const flashUsage = Math.floor((data.disk.root.used / data.disk.root.total) * 100)

      // Formating rows for card
      const propedRows = [
        ['CPU LOAD', cpuLoad, 'progress-bar'],
        ['ROUTER UPTIME', '%t'.format(data.uptime)],
        ['LOCAL DEVICE TIME', this.toDate(data.localtime)],
        ['MEMORY USAGE', memUsage, 'progress-bar'],
        ['FLASH USAGE', flashUsage, 'progress-bar'],
        ['FIRMWARE VERSION', data.release.revision]
      ]

      this.addPropsAllRows(propedRows)

      const card = {
        title: 'SYSTEM',
        rows: propedRows
      }

      return card
    },
    /**
     * Gets device cpu load
     * @return {int} cpu load
     */
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
    /**
     * Gets an array of cards containing each interface data
     * @return {array} array of cards
     */
    async getInterfacesCards () {
      return await this.$network.load().then(() => {
        const ifaces = this.$network.getInterfaces()

        // Formating cards
        const cards = []

        ifaces.forEach(iface => {
          const propedRows = [
            ['TYPE', 'wired'],
            ['IP ADDRESS', iface.getIPv4Addrs().join(' ')]
          ]
          this.addPropsAllRows(propedRows)

          cards.push({
            title: iface.name,
            rows: propedRows
          })
        })

        return cards
      })
    },
    /**
     * Gets an array of cards containing provided event data
     * @param {*} type event type
     * @param {*} lim limit event to retrieve
     * @return {array} array of cards
     */
    async getEventsCard (type, lim) {
      return this.$log.get({ table: type, limit: lim }).then((r) => {
        // Formatting rows for card
        const propedRows = []
        r.log.forEach((event) => {
          propedRows.push([
            this.toDate(event.TIME),
            event.TEXT
          ])
        })
        this.addPropsAllRows(propedRows)

        return {
          title: `${type} EVENTS`,
          rows: propedRows
        }
      })
    },
    /**
     * Converts timestamp to datetime
     * @param {timestamp} timestamp timestamp to convert
     * @return {datetime}
     */
    toDate (timestamp) {
      let localDate = new Date(timestamp * 1000)
      localDate = localDate.toLocaleDateString('lt-LT') + ' ' + localDate.toLocaleTimeString('lt-LT')
      return localDate
    }
  },
  async created () {
    this.cardsData = await this.getCardsData()
  }
}
</script>

<style>
.cards-wrapper {
  display: inline-block;
}
</style>
