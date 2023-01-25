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
  data () {
    return {
      isChecked: 0
    }
  },
  methods: {
    changed () {
      return this.initialValue !== this.isChecked.toString()
    },
    __save () {
      this.isChecked = this.isChecked ? 1 : 0

      if (this.changed()) {
        // console.log('pasikeite')
        this.$uci.set('mosquitto', this.sid, this.name, this.isChecked.toString())
      } else {
        // console.log('nepasikeite')
      }
      // console.log(this.initialValue)
      // console.log(this.model)
      // console.log(this.isChecked)
    }
  },
  created () {
    if (this.model === '1') { this.isChecked = 1 }
  }
}
</script>
