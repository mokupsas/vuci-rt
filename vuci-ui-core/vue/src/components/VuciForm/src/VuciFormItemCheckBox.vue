<template>
  <vuci-form-item-template v-bind="VuciFormItemTemplateProps">
    <!-- <input type="checkbox" v-model.lazy="isChecked" :checked="isChecked"/> -->
    <a-checkbox v-model.lazy="isChecked"></a-checkbox>
  </vuci-form-item-template>
</template>

<script>
import VuciFormItemMixin from './VuciFormItemMixin.vue'

/* Docs
https://1x.antdv.com/components/checkbox/
*/

export default {
  name: 'VuciFormItemCheckBox',
  mixins: [VuciFormItemMixin],
  props: {
    autoFocus: { default: false },
    disabled: { default: false }
  },
  data () {
    return {
      isChecked: false
    }
  },
  methods: {
    boolToNumStr (bool) {
      if (bool) return '1'
      return '0'
    },
    changed () {
      return this.initialValue !== this.boolToNumStr(this.isChecked)
    },
    __save () {
      if (this.changed()) { this.$uci.set('mosquitto', this.sid, this.name, this.boolToNumStr(this.isChecked)) }
    }
  },
  created () {
    if (this.model === '1') { this.isChecked = true }
  }
}
</script>
