<template>
    <div class="example">

        <a-button v-if="!alert" class="visibility-toggle-btn" type="primary" @click="openDrawer">
          Change visibility
        </a-button>

        <a-alert v-show="alert" message="No cards were found" type="warning" show-icon />

        <a-drawer
          title="Card visibility"
          placement="right"
          :closable="true"
          :visible="showDrawer"
          @close="openDrawer"
        >
          <div v-for="(card, key) in cardsData" :key="key">
            <input type="checkbox" :name="card.title" v-on:change="changeConfigVisibility(key)" v-model="card.visible"/>
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
              :cpuLoad="getCardCpuLoad(card)"
            ></card>
          </draggable>
        </div>

    </div>
</template>

<script>
import draggable from 'vuedraggable'
import Card from './Card.vue'

// Adds properties to card row array values
const addCardRowProperties = obj => {
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
      alert: false, // alert message, that no cards exist
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
     * Search card object by title in given array
     * @param {array} array array to search in
     * @param {string} searchTitle card title to search for
     */
    findCardByTitle (array, searchTitle) {
      const findCard = array.find(card => card.title === searchTitle)
      if (findCard) { return findCard }
      return false
    },
    /**
     * Called when draggable object has been moved to new position
     * @param {object} event moved object event data
     */
    async changeCardPosition (event) {
      const configs = await this.getAllConfigs()

      // Swaping cards
      this.cardsData = this.moveArrayEl(this.cardsData, event.oldIndex, event.newIndex)

      // After moving one card, position migth change for multiple cards.
      // So we save position of all cards
      for (let i = 0; i < this.cardsData.length; i++) {
        const sid = this.findCardByTitle(configs, this.cardsData[i].title)['.name']
        this.$uci.set('overview', sid, 'position', i)
      }
      await this.$uci.save()
      await this.$uci.apply()
    },
    /**
     * Change element place in array. Swaps every other element in between changed positions
     * @param {array} arr array to move elements in
     * @param {int} index index of element to move
     * @param {int} moveTo index where to move element
     */
    moveArrayEl (arr, index, moveTo) {
      // In case we swap element backwards, we make index to have the smallest value
      const swapArr = [index, moveTo].sort((x, y) => x - y) // array of index and moveTo ascending
      index = swapArr[0]
      moveTo = swapArr[1]

      arr.splice(moveTo + 1, 0, arr[index]) // move to position
      arr.splice(index, 1).join() // remove moved item (prevents duplication)
      return arr
    },
    /**
     * Changes card uci config visibility. Called on checkbox click
     * @param {int} index card index in cardsData array
     */
    async changeConfigVisibility (index) {
      const configs = await this.getAllConfigs()
      const findConfig = configs.find(card => card.position === index.toString())

      if (findConfig) {
        const visible = parseInt(findConfig.visible)

        this.$uci.set('overview', findConfig['.name'], 'visible', visible ? 0 : 1)
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
      cards.push(await this.getSysCard())

      // Interface cards
      const intCards = await this.getInterfaceCards()
      intCards.forEach(card => cards.push(card))

      // Network events card
      cards.push(await this.getEventsCard('NETWORK', 5))

      // System events card
      cards.push(await this.getEventsCard('SYSTEM', 5))

      cards = await this.addCardProperties(cards) // gets position and visibility props
      cards = this.sortCardsByPos(cards) // sorts cards by their position
      return cards
    },
    /**
     * Gets an array of card objects, which have changed
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
     * Adds properties to each card row in array and returns be reference
     * @param {array} rows
     */
    addCardRowsProps (rows) {
      // Adding properties to each row
      rows.forEach(row => {
        addCardRowProperties(row)
      })
    },
    /**
     * Creates uci config for each card that doesn't exist in array
     * @param {array} cards array of cards
     * @return {bool} was atleast one config created
     */
    async createConfigFromArray () {
      let wasCreated = false // Checks if atleast one config has been created
      const configs = await this.getAllConfigs()
      let pos = configs.length // count card position (starting from number of config entries)

      this.cardsData.forEach(card => {
        const config = this.findCardByTitle(configs, card.title)

        if (config) { return } // if config found, skip iteration

        // If uci config for specific card doesn't exist, create one
        this.createCardConfig(card.title, pos, true)
        wasCreated = true
        pos++ // increasing pos after adding uci config
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
     * Adds position and visibility properties and value to each cards from uci config
     * @param {array} cards array of cards
     * @return {array} array with new properties
     */
    async addCardProperties (cards) {
      const configs = await this.getAllConfigs()

      cards.forEach((card) => {
        const findConfig = this.findCardByTitle(configs, card.title)

        if (!findConfig) { return } // if no data found skip iteration

        // Update cardsData with properties and config data
        card.position = parseInt(findConfig.position)
        if (findConfig.visible === '1') {
          card.visible = true
        } else {
          card.visible = false
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
        ['ROUTER UPTIME', '%t'.format(data.uptime)],
        ['LOCAL DEVICE TIME', this.toDate(data.localtime)],
        ['MEMORY USAGE', memUsage, 'progress-bar'],
        ['FLASH USAGE', flashUsage, 'progress-bar'],
        ['FIRMWARE VERSION', data.release.revision]
      ]

      this.addCardRowsProps(propedRows)

      const card = {
        title: 'SYSTEM',
        rows: propedRows,
        cpuLoad: cpuLoad
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
     * Gets an array of cards, each containing interface data
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
          this.addCardRowsProps(propedRows)

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
        this.addCardRowsProps(propedRows)

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
     * Creates uci config entry for card
     * @param {string} title card title
     * @param {int} pos card position
     * @param {bool} visib card visible
     */
    createCardConfig (title, pos, visib) {
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
     * Gets config of all cards
     * @return {array} array of configs
     */
    async getAllConfigs () {
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
    /**
     * Removes non existing cards data from uci config
     */
    async removeNonExistingConf () {
      const configs = await this.getAllConfigs()

      configs.forEach(config => {
        const findCard = this.findCardByTitle(this.cardsData, config.title)

        if (findCard) { return } // if card found, skip iteration

        this.$uci.del('overview', config['.name'])
      })
      await this.$uci.save()
      await this.$uci.apply()
    },
    /**
     * Checks if cards exist
     * @return {bool} if atleast one card exist, returns true
     */
    doesCardsExist () {
      if (this.cardsData[0] && Object.hasOwn(this.cardsData[0], 'visible')) {
        return true
      }
      return false
    },
    /**
     * Checks if card has cpuLoad property, if so returns its value
     * @param {object} card card object
     * @return {int|bool} cpu load | false on no property
     */
    getCardCpuLoad (card) {
      if (Object.hasOwn(card, 'cpuLoad')) {
        return card.cpuLoad
      }
      return false
    }
  },
  async created () {
    // Fetch each card data
    this.cardsData = await this.getCardsData()

    // If atleast one uci config has been created, we reload cardsData
    // to see changes instantly
    if (await this.createConfigFromArray()) {
      this.cardsData = await this.getCardsData()
    }

    // Removes uci configs, for cards which doesn't exist
    this.removeNonExistingConf()

    // Shows notification if no cards to show
    this.alert = !this.doesCardsExist()
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
