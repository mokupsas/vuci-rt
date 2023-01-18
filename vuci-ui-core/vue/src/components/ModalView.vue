<template>
    <a-modal v-model="show" :title="modalTitle(this.title)" @cancel="$emit('cancel')">
        <a-form-model>
            <vuci-form v-if="show" uci-config="vuci_components_task" @applied="$emit('applied')">
                <vuci-named-section v-if="show" :name="this.uci" v-slot="{ s }">
                    <vuci-form-item-select @change="optionChange" v-model="selectedProto" :uci-section="s" :options="protocols" :label="$t('Protocol')" name="proto"/>
                      <vuci-form-item-input :uci-section="s" :label="$t('Address')" name="address" depend="proto == 'static'" rules="ip4addr"/>
                      <vuci-form-item-input :uci-section="s" :label="$t('Netmask')" name="netmask" depend="proto == 'static'" rules="netmask4"/>
                      <vuci-form-item-input :uci-section="s" :label="$t('Gateway')" name="gateway" depend="proto == 'static'"/>
                      <vuci-form-item-list :uci-section="s" :label="$t('DNS')" name="dns" depend="proto == 'static'"/>
                </vuci-named-section>

                <!-- <template slot="footer"><div></div></template> -->
            </vuci-form>
        </a-form-model>
        <template #footer><div/></template>
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
    modalTitle (name) {
      return `Interface ${name}`
    },
    optionChange (self) {
      this.selectedProto = self.model
    }
  },
  created () {
    console.log(this.uci)
  }
}
</script>

<style>

</style>
