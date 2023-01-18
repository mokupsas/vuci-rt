<template>

    <div>
        <vuci-form uci-config="vuci_components_task" :key="dataChange">
            <vuci-typed-section type="interface" :columns="columns" :add="addItem">
                <template #name="{ s }">
                    <vuci-form-item-dummy :uci-section="s"  name="name"/>
                </template>
                <template #address="{ s }">
                    <vuci-form-item-dummy :uci-section="s"  name="address" rules="ipaddr"/>
                </template>
                <template #netmask="{ s }">
                    <vuci-form-item-dummy :uci-section="s"  name="netmask" rules="netmask4"/>
                </template>
                <template #proto="{ s }">
                    <a-button type="primary" style="margin-right: 10px" size="small" @click="editItem(s)">{{ $t('Edit') }}</a-button>

                    <a-popconfirm @confirm="deleteItem(s['.name'])" placement="left" :title="$t('interfaces.DelConfirm')">
                      <a-button type="danger" size="small">{{ $t('Delete') }}</a-button>
                    </a-popconfirm>

                </template>
            </vuci-typed-section>
            <label class="field-label" for="enable">Interface name</label>
            <div class="input-wrapper">
              <input type="text" v-model="newInterface" class="ant-input">
            </div>
            <button class="ant-btn ant-btn-primary ant-btn-sm" style="margin-right: 10px;" @click="addItem">Create</button>
            <template slot="footer"><div></div></template>
        </vuci-form>

        <modalView v-if="modalShow" :title="this.modalTitle" :uci="this.selectedUciName" @cancel="handleCancel" @applied="realoadData"></modalView>

    </div>

</template>

<script>
export default {
  data () {
    return {
      columns: [
        { name: 'name', label: 'Interface name' },
        { name: 'address', label: 'Address' },
        { name: 'netmask', label: 'Netmask' },
        { name: 'proto', label: '' }
      ],
      dataChange: 1,
      // Modal
      modalShow: false,
      modalTitle: null,
      // Section that we edit
      selectedUciName: null,
      // Adding new section
      addName: null,
      newInterface: null
    }
  },
  methods: {
    async load () {
      await this.$uci.load('vuci_components_task')
    },
    async realoadData () {
      await this.load()
      this.dataChange++
    },
    editItem (s) {
      this.modalTitle = s.name
      this.selectedUciName = s['.name']
      this.modalShow = true
    },
    async getTypedSection (id) {
      await this.$uci.load('vuci_components_task')
      const interfaces = this.$uci.sections('vuci_components_task', 'interface')
      return interfaces[id]
    },
    async addItem () {
      await this.load()

      // Checking if input is empty
      if (!this.newInterface) {
        console.log('Name is empty')
        return false
      }

      // Checking if interface already exsit
      if (await this.doesInterfaceExist(this.newInterface)) {
        console.log('The name already exist')
        return false
      }

      const sid = await this.$uci.add('vuci_components_task', 'interface') // creating new interface
      await this.$uci.set('vuci_components_task', sid, 'name', this.newInterface) // adding name to the interface
      await this.$uci.save()
      await this.$uci.apply()
      const section = await this.findNewestSection() // fetching newly created interface .name
      this.modalTitle = this.newInterface // title for
      this.selectedUciName = section
      // Open modal
      this.modalShow = true
    },
    // iName - interface name
    async deleteItem (iName) {
      this.$uci.del('vuci_components_task', iName)
      await this.$uci.save()
      await this.$uci.apply()
      this.realoadData()
    },
    async doesInterfaceExist (name) {
      await this.$uci.load('vuci_components_task')
      const sections = await this.$uci.sections('vuci_components_task', 'interface')
      const count = sections.length

      for (let i = 0; i < count; i++) {
        if (name === sections[i].name) { return true }
      }

      return false
    },
    // Returns section name
    async findNewestSection () {
      await this.$uci.load('vuci_components_task')
      const sections = await this.$uci.sections('vuci_components_task', 'interface')
      return sections[sections.length - 1]['.name']
    },
    handleCancel () {
      this.modalShow = false
    }
  }
}
</script>

<style>
.input-wrapper {
  display: inline-table;
  width: 200px;
  margin: 0 10px 0 10px;
}
</style>
