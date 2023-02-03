<template>
    <a-modal v-model="show" :title="modalTitle(this.title)" @cancel="$emit('cancel')">
        <a-form-model>
            <vuci-form v-if="show" uci-config="vuci_components_task" @applied="$emit('applied')">
                <vuci-named-section v-if="show" :name="this.uci" v-slot="{ s }">
                    <vuci-form-item-select :save="saveProto" initial="static" @change="optionChange" v-model="selectedProto" :uci-section="s" :options="protocols" :label="$t('Protocol')" name="proto"/>
                      <vuci-form-item-input :uci-section="s" :label="$t('Address')" name="address" depend="proto == 'static'" :rules="{ type: 'ip4addr' }" required/>
                      <vuci-form-item-input :uci-section="s" :label="$t('Netmask')" name="netmask" depend="proto == 'static'" :rules="{type: 'netmask4', min: 10}" required/>
                      <vuci-form-item-input :uci-section="s" :label="$t('Gateway')" name="gateway" depend="proto == 'static'"/>
                      <vuci-form-item-list :uci-section="s" :label="$t('DNS')" name="dns" depend="proto == 'static'"/>
                </vuci-named-section>

                <template slot="footer"></template>
            </vuci-form>
        </a-form-model>
        <template #footer><div></div></template>
    </a-modal>
</template>

<script>
export default {
  name: 'ModalView',
  props: {
    title: { default: null },
    uci: { required: true }
  },
  data () {
    return {
      show: true,
      protocols: [
        ['static', 'Static'],
        ['dhcp', 'DHCP']
      ],
      selectedProto: null
    }
  },
  methods: {
    saveProto (self) {
      // If protocol is DHCP
      if (self.model === 'dhcp') {
        this.$uci.del('vuci_components_task', self.sid, 'address')
        this.$uci.del('vuci_components_task', self.sid, 'netmask')
        this.$uci.del('vuci_components_task', self.sid, 'gateway')
        this.$uci.del('vuci_components_task', self.sid, 'dns')
      }

      // Saving protocol value
      this.$uci.set('vuci_components_task', self.sid, 'proto', self.model)
    },
    modalTitle (name) {
      return `Interface ${name}`
    },
    optionChange (self) {
      this.selectedProto = self.model
    },
    validate (value) {
      if (value.length > 0) { return }
      if (!value.length) { return }

      return this.$t('validator.integer')
    }
  }
}
</script>

<style>

</style>
