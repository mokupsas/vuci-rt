<template>
  <vuci-form-item-template v-bind="VuciFormItemTemplateProps">
    <component :is="is" v-model="model" :type="type" :placeholder="placeholder" :maxLength="maxLength" :prefix="prefixIcon" :suffix="suffixIcon"
    :addon-before="prepend" :addon-after="append" @change="test"/>
  </vuci-form-item-template>
</template>

<script>
import VuciFormItemMixin from './VuciFormItemMixin'

export default {
  name: 'VuciFormItemInput',
  mixins: [VuciFormItemMixin],
  props: {
    placeholder: String,
    maxLength: Number,
    prepend: String,
    append: String,
    type: {
      validator (value) {
        return ['text', 'url', 'email', 'date', 'number', 'tel'].indexOf(value) !== -1
      },
      default: 'text'
    },
    password: Boolean,
    prefixIcon: String,
    suffixIcon: String
  },
  methods: {
    test () {
      console.log(this.changed())
    }
  },
  computed: {
    is () {
      if (this.password) return 'a-input-password'
      return 'a-input'
    }
  }
}
</script>
