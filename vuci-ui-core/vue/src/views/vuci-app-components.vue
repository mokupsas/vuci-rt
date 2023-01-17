<template>

    <!--
    <a-table :columns="table.columns" :data-source="table.data" :loading="table.loading">
        <template>
            <a-button>a</a-button>
        </template>
    </a-table>
    -->

    <!--
    <vuci-form uci-config="vuci_components_task">
        <vuci-typed-section :teasers="['name']" addremove type="interface" v-slot="{ s }">
            <template>
                <vuci-form-item-dummy :uci-section="s" :label="$t('Interface name')" name="name"/>

                <vuci-form-item-select :uci-section="s" :label="$t('Protocol')" name="proto" :options="[ 'static', 'dhcp']"/>

                <vuci-form-item-input :uci-section="s" :label="$t('Address')" name="address"/>

                <vuci-form-item-input :uci-section="s" :label="$t('Netmask')" name="netmask"/>

                <vuci-form-item-input :uci-section="s" :label="$t('Gateway')" name="gateway"/>

                <vuci-form-item-list :uci-section="s" :label="$t('DNS')" name="dns"/>
            </template>
        </vuci-typed-section>
    </vuci-form>
    -->

    <div>
        <vuci-form uci-config="vuci_components_task">
            <vuci-typed-section type="interface" :columns="columns" addremove :add="addItem">
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
                    <a-button type="danger" style="margin-right: 10px" size="small">{{ $t('Delete') }}</a-button>
                </template>
            </vuci-typed-section>
            <label class="field-label" for="enable">Interface name</label>
            <div class="field-input">
              <input type="text" class="input">
            </div>
            <!-- <template slot="footer"><div></div></template> -->
        </vuci-form>

        <!--
        <a-modal v-if="selectedSection" v-model="modalShow" :title="modalTitle">
            <a-form-model >
                <a-form-model-item label="Protocol">
                    <a-input v-model="selectedSection.proto"/>
                </a-form-model-item>
                <a-form-model-item label="Address">
                    <a-input v-model="selectedSection.address"/>
                </a-form-model-item>
                <a-form-model-item label="Netmask">
                    <a-input v-model="selectedSection.netmask"/>
                </a-form-model-item>
                <a-form-model-item label="Gateway">
                    <a-input v-model="selectedSection.gateway"/>
                </a-form-model-item>
                <a-form-model-item label="DNS">
                    <a-input v-model="selectedSection.dns"/>
                </a-form-model-item>
            </a-form-model>
        </a-modal>
        -->

        <a-modal v-model="modalShow" :title="modalTitle(this.selectedName)">
            <a-form-model >
                <vuci-form uci-config="vuci_components_task">

                    <vuci-named-section :name="this.selectedUciName" v-if="modalShow" v-slot="{ s }">
                        <vuci-form-item-select :uci-section="s" :options="protocols" :label="$t('Protocol')" name="proto"/>
                        <vuci-form-item-input :uci-section="s" :label="$t('Address')" name="address"/>
                        <vuci-form-item-input :uci-section="s" :label="$t('Netmask')" name="netmask"/>
                        <vuci-form-item-input :uci-section="s" :label="$t('Gateway')" name="gateway"/>
                        <vuci-form-item-list :uci-section="s" :label="$t('DNS')" name="dns"/>
                    </vuci-named-section>

                    <!-- <template slot="footer"><div></div></template> -->
                </vuci-form>
            </a-form-model>
            <template #footer><div/></template>
        </a-modal>

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
      protocols: [
        ['static', 'Static'],
        ['dhcp', 'DHCP']
      ],
      // Modal
      modalShow: false,
      // Sectio that we edit
      selectedSection: null,
      selectedName: null,
      selectedUciName: null
    }
  },
  methods: {
    get_time () {
      this.$rpc.call('example', 'get_time', {}).then(r => {
        this.time = r.time
      })
    },
    editItem (s) {
      this.selectedName = s.name
      this.selectedUciName = s['.name']
      // this.selectedSection = await this.getTypedSection(s.key)
      this.modalShow = true
      console.log(s)
    },
    async getTypedSection (id) {
      await this.$uci.load('vuci_components_task')
      const interfaces = this.$uci.sections('vuci_components_task', 'interface')
      return interfaces[id]
    },
    modalTitle (name) {
      return `Interface ${name}`
    },
    addItem (self) {
      this.$prompt({
        title: 'Add',
        placeholder: 'Please input a name',
        validator: value => {
          if (self.sections.filter(s => s.name === value).length > 0) {
            return 'The name already exist'
          }
        }
      }).then(value => {
        const sid = this.addSection('interface')
        this.$uci.set('vuci_components_task', sid, 'name', value)
        this.saveUci()
      })
    },
    addSection (type, name) {
      const sid = this.$uci.add('vuci_components_task', type, name)
      return sid
    },
    saveUci () {
      this.$uci.save().then(() => {
        this.$uci.apply()
      })
    }
  },
  created () {
    // console.log(this.addSection('interface'))
  }
}
</script>

<style>
</style>
