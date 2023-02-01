<template>
    <div class="example">

        <a-button v-if="cardsData.length !== 0" class="visibility-toggle-btn" type="primary" @click="openDrawer">
          Change visibility
        </a-button>

        <a-alert v-if="cardsData.length === 0" message="No cards were found" type="warning" show-icon />

        <a-drawer
          title="Card visibility"
          placement="right"
          :closable="true"
          :visible="showDrawer"
          @close="openDrawer"
        >
          <div v-for="(card, key) in cardsData" :key="key">
            <input type="checkbox" :name="card.title" v-on:change="changeCardVisibility(key)" v-model="card.visible"/>
            <label :for="card.title"> {{ card.title }}</label>
          </div>
        </a-drawer>

        <div class="cards-wrapper">
          <draggable v-model="cardsData" :component-data="getDraggableData()">
            <card
              v-for="(card, key) in cardsData" :key="key"
              :title="card.title"
              :data="card.rows"
              v-show="card.visible"
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
          visible: true
        }
        */
      ]
    }
  },
  timers: {
    fetchAndUpdateCardChanges: { time: 1000, autostart: true, immediate: false, repeat: true }
  },
  methods: {
    /**
     * Manages draggable object data
     */
    getDraggableData () {
      return {
        on: {
          change: this.changeCardPosition
        }
      }
    },
    /**
     * Called when draggable object has been moved to new position
     * @param {object} event moved object event data
     */
    async changeCardPosition (event) {
      const oldIndex = event.oldIndex // moved card old index
      const newIndex = event.newIndex // moved card new index
      const ifaces = await this.getCardInterfaces()
      let cardsCopy = this.cardsData

      // We move a copy of array elements, because cardsData is being updated after this  method
      cardsCopy = this.moveArrayEl(cardsCopy, oldIndex, newIndex)

      // After moving one card, position migth change for multiple cards.
      // So we save position of cards, whose position has changes
      for (let i = 0; i < cardsCopy.length; i++) {
        const sid = ifaces.find(card => card.title === cardsCopy[i].title)['.name'] // getting current card sid
        this.$uci.set('overview', sid, 'position', i)
      }
      await this.$uci.save()
      await this.$uci.apply()
    },
    /**
     * Move element in array
     * @param {array} arr array to move elements in
     * @param {int} index index of element to move
     * @param {int} moveTo index where to move element
     */
    moveArrayEl (arr, index, moveTo) {
      if (moveTo > index) {
        arr.splice(moveTo + 1, 0, arr[index]) // move to position
        arr.splice(index, 1) // remove moved item (prevents duplication)
      } else {
        arr.splice(index + 1, 0, arr[moveTo]) // move to position
        arr.splice(moveTo, 1) // remove moved item (prevents duplication)
      }
      arr.join()
      return arr
    },
    /**
     * Changes card uci config visibility. Called on checkbox click
     * @param {int} index card index in cardsData array
     */
    async changeCardVisibility (index) {
      const ifaces = await this.getCardInterfaces()
      const findCard = ifaces.find(card => card.position === index.toString())

      if (findCard) {
        const visible = parseInt(findCard.visible)

        this.$uci.set('overview', findCard['.name'], 'visible', visible ? 0 : 1)
        await this.$uci.save()
        await this.$uci.apply()
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

      cards = await this.addCardProperties(cards) // gets position and visibility props
      if (!cards) return [] // if couldn't load properties from uci

      cards = this.sortCardsByPos(cards) // sorts cards by their position

      return cards
    },
    /**
     * Gets an array of objects, that have changed between two arrays
     * @param {array} card1 array of cards
     * @param {array} card2 array of cards to compare against
     * @return {array|bool} array of cards | false if no cards found
     */
    getChangesBetweenCards (card1, card2) {
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
     * Creates interfaces for each card that doesn't exist in array
     * @param {array} cards array of cards
     * @return {bool} was atleast one interface created
     */
    async createInterfacesFromArray (cards) {
      let wasCreated = false // Checks if atleast one interfaces has been created
      const config = await this.getCardInterfaces()
      let pos = config.length // count card position (starting from number of config entries)

      // Checks if card has config interface
      cards.forEach(card => {
        const iface = config.find(conf => conf.title === card.title)

        // If config interface doesn't exist, create one
        if (!iface) {
          this.createCardInterface(card.title, pos, true)
          wasCreated = true
          pos++ // increasing pos after adding config interface
        }
      })
      // Save uci changes
      await this.$uci.save()
      await this.$uci.apply()

      return wasCreated
    },
    /**
     * Fetches and updates each card data. Called by timing
     */
    async fetchAndUpdateCardChanges () {
      const cards = await this.getCardsData()
      const changedCards = this.getChangesBetweenCards(cards, this.cardsData)

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

      let pos = 0 // count card position
      this.cardsData.forEach(async (card) => {
        const iface = config.find(conf => conf.title === card.title)

        // If config interface doesn't exist, create one
        if (!iface) {
          this.createCardInterface(card.title, pos, card.visible)
        } else {
          if (pos !== iface.position) {
            this.$uci.set('overview', iface['.name'], 'position', pos)
          }
          if (card.visible !== iface.visible) {
            this.$uci.set('overview', iface['.name'], 'visible', card.visible ? 1 : 0)
          }
        }

        pos++
      })
      // Save uci changes
      await this.$uci.save()
      await this.$uci.apply()
    },
    /**
     * Adds position and visibility properties to cards array from uci config interface
     * @param {array} cards array of cards
     * @return {array} array with new properties
     */
    async addCardProperties (cards) {
      const config = await this.getCardInterfaces()
      if (config.length === 0) return false

      cards.forEach((card) => {
        // If config interface doesn't exist, create one
        const iface = config.find(conf => conf.title === card.title)

        // Update cardsData with config data
        if (iface) {
          card.position = parseInt(iface.position)
          if (iface.visible === '1') {
            card.visible = true
          } else {
            card.visible = false
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
     * @return {datetime} converted datetime
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
      this.$uci.set('overview', sid, 'visible', visib ? 1 : 0)

      return true
    },
    /**
     * Gets cards uci config interfaces
     * @return {array} array of interfaces
     */
    async getCardInterfaces () {
      return await this.$uci.load('overview').then(async () => {
        return await this.$uci.sections('overview', 'interface')
      })
    },
    /**
     * Opens drawer which contains card visbility change checkboxes
     */
    openDrawer () {
      this.showDrawer = !this.showDrawer
    },
    async removeNonExistingConf () {
      const config = await this.getCardInterfaces()

      config.forEach(iface => {
        const findCard = this.cardsData.find(card => card.title === iface.title)
        if (!findCard) {
          this.$uci.del('overview', iface['.name'])
        }
      })
      await this.$uci.save()
      await this.$uci.apply()
    }
  },
  async created () {
    // Fetch each card data
    this.cardsData = await this.getCardsData()

    // If atleast one interface has been created, we reload cardsData
    if (await this.createInterfacesFromArray(this.cardsData)) {
      this.cardsData = await this.getCardsData()
    }

    this.removeNonExistingConf()
    console.log(this.cardsData.length)
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
