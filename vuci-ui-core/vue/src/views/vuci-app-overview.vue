<template>
    <div class="example">

        <a-button class="visibility-toggle-btn" type="primary" @click="openDrawer">
          Change visibility
        </a-button>

        <a-drawer
          title="Card visibility"
          placement="right"
          :closable="true"
          :visible="showDrawer"
          @close="openDrawer"
        >
          <div v-for="(card, key) in cardsData" :key="key">
            <input type="checkbox" :name="card.title" v-on:change="changeCardVisibility(key)" v-model="card.visibility"/>
          <!-- <!-- <div> -->
            <!-- <a-checkbox v-for="(card, key) in cardsData" :key="key" :name="card.title" @change="test(card, key)" v-model="card.visibility"> {{ card.title }}</a-checkbox> -->
            <label :for="card.title"> {{ card.title }}</label>
          </div>
        </a-drawer>

        <div class="cards-wrapper">
          <draggable v-model="cardsData" :component-data="getDraggableData()">
            <card
              v-for="(card, key) in cardsData" :key="key"
              :title="card.title"
              :data="card.rows"
              v-show="card.visibility"
            ></card>
          </draggable>
        </div>

    </div>
</template>

<script>
import draggable from 'vuedraggable'
import Card from './Card.vue'

// Adds properties to row array values
const addRowProperties = obj => {
  obj.title = obj[0]
  obj.value = obj[1]
  obj.type = obj[2]
}

// Adds positioning and visibility properties to card object

export default {
  name: 'Overview',
  components: { Card, draggable },
  data () {
    return {
      showDrawer: false,
      cardsData: [
        /*
        {
          title: 'SYSTEM',
          rows: [
            ['CPU load', 50],
            ['CPU load1', 50]
          ],
          position: 0,
          visibility: true
        }
        */
      ]
    }
  },
  timers: {
    // update: { time: 1000, autostart: true, immediate: false, repeat: true },
    // configSaveCardChanges: { time: 1000, autostart: true, immediate: false, repeat: true }
  },
  methods: {
    getDraggableData () {
      return {
        on: {
          change: this.changeCardPosition
        }
      }
    },
    async changeCardPosition (event) {
      const oldIndex = event.oldIndex // used to gather card
      const newIndex = event.newIndex // used to store moved card new position

      // Getting switched cards interfaces
      const ifaces = await this.getCardInterfaces()
      const findMovedCard = ifaces.find(card => card.title === this.cardsData[oldIndex].title)
      const findAnotherCard = ifaces.find(card => card.title === this.cardsData[newIndex].title)

      // Save positions to UCI config file
      if (findMovedCard && findAnotherCard) {
        this.$uci.set('overview', findMovedCard['.name'], 'position', newIndex)
        this.$uci.set('overview', findAnotherCard['.name'], 'position', oldIndex)
        this.$uci.save()
        this.$uci.apply()
      }
    },
    test (e) {
      console.log(e)
    },
    async changeCardVisibility (index) {
      const ifaces = await this.getCardInterfaces()
      const findCard = ifaces.find(card => card.position === index.toString())

      if (findCard) {
        const visible = parseInt(findCard.visibility)

        this.$uci.set('overview', findCard['.name'], 'visibility', visible ? 0 : 1)
        this.$uci.save()
        this.$uci.apply()
      }
    },
    /**
     * Gets an array of cards
     */
    async getCardsData () {
      let cards = []

      // System card
      const sysCard = await this.getSysCard()
      cards.push(sysCard)

      // Interface cards
      const intCards = await this.getInterfaceCards()
      intCards.forEach(card => cards.push(card))

      // Network events card
      const netEvCard = await this.getEventsCard('NETWORK', 5)
      cards.push(netEvCard)

      // System events card
      const sysEvCard = await this.getEventsCard('SYSTEM', 5)
      cards.push(sysEvCard)

      cards = await this.getCardConfigs(cards)
      cards = this.sortCardsByPos(cards)

      return cards
    },
    /**
     * Gets an array of objects, that have changed between two arrays
     * @param {array} card1 array of cards
     * @param {array} card2 array of cards to compare against
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
     * Fetches and updates each card data. Called by timing
     */
    async update () {
      const cards = await this.getCardsData()
      const changedCards = this.getCardsChanges(cards, this.cardsData)

      // Check if any card has been changed
      if (changedCards) {
        // Update cards, whose data has been changed
        changedCards.forEach(card => {
          const index = this.cardsData.findIndex(item => item.title === card.title)
          this.cardsData[index].rows = card.rows
        })
      }
    },
    /**
     * Update uci config with cards data (position, visibility)
     */
    async configSaveCardChanges () {
      const config = await this.getCardInterfaces()

      let pos = 0
      this.cardsData.forEach(async (card) => {
        const iface = config.find(conf => conf.title === card.title)

        // If config interface doesn't exist, create one
        if (!iface) {
          this.createCardInterface(card.title, pos, card.visibility)
        }
        // If config exists, update if required
        else {
          if (pos !== iface.position) {
            this.$uci.set('overview', iface['.name'], 'position', pos)
          }
          if (card.visibility !== iface.visibility) {
            this.$uci.set('overview', iface['.name'], 'visibility', card.visibility ? 1 : 0)
          }
        }

        pos++
      })
      // Save uci changes
      this.$uci.save()
      this.$uci.apply()
    },
    /**
     * Loads uci config data and saves it into cardsData
     */
    async getCardConfigs (cards) {
      const config = await this.getCardInterfaces()

      cards.forEach((card) => {
        // If config interface doesn't exist, create one
        const iface = config.find(conf => conf.title === card.title)

        // Update cardsData with config data
        if (iface) {
          card.position = parseInt(iface.position)
          if (iface.visibility === '1') {
            card.visibility = true
          } else {
            card.visibility = false
          }
        }
      })
      return cards
    },
    /**
     * Gets system data and retuns an object
     * @return {object} system data object
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
    async getInterfaceCards () {
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
     * @param {string} type event type
     * @param {int} lim limit event to retrieve
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
    },
    /**
     * Sort cards array by position (ascending order)
     * @param {array} cardsData array of cards
     */
    sortCardsByPos (array) {
      return array.sort((x, y) => {
        if (x.position < y.position) {
          return -1
        }
        if (x.position > y.position) {
          return 1
        }
        return 0
      })
    },
    /**
     * Creates config interface for card
     * @param {string} title card title
     * @param {int} pos card position
     * @param {bool} visib card visible
     */
    createCardInterface (title, pos, visib) {
      pos = pos || 0
      visib = visib ? 1 : 0

      if (!title && !Number.isInteger(pos)) return false

      const sid = this.$uci.add('overview', 'interface')
      this.$uci.set('overview', sid, 'title', title)
      this.$uci.set('overview', sid, 'position', pos)
      this.$uci.set('overview', sid, 'visibility', visib ? 1 : 0)

      return true
    },
    /**
     * Gets all config interfaces
     * @return {array} array of interfaces
     */
    async getCardInterfaces () {
      return await this.$uci.load('overview').then(async () => {
        return await this.$uci.sections('overview', 'interface')
      })
    },
    openDrawer () {
      this.showDrawer = !this.showDrawer
    }
  },
  async created () {
    // Fetch each card data
    this.cardsData = await this.getCardsData()
    // console.log(this.cardsData)

    // Load config properties (position, visibility into cardsData)
    // await this.loadUciConfig()

    // Sorting data by position
    // this.sortCardsByPos()
  }
}
</script>

<style>
.cards-wrapper {
  display: inline-block;
}
.visibility-toggle-btn {
  float: right;
}
</style>
