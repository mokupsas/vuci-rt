<template>
    <a-card class="card" :title="this.title">
        <div class="row" v-for="(item, key) in data" :key="key">
            <strong>{{ item.title }}</strong>
            <br>
            <component :is="componentByType(item.type)" :percent="item.value" status="normal">{{ item.value }}</component>
        </div>

      <slot></slot>
    </a-card>
</template>

<script>
export default {
  name: 'Card',
  props: {
    title: { required: true },
    data: { required: true }
  },
  methods: {
    componentByType (type) {
      if (type === 'progress-bar') return 'a-progress'
      return 'span'
    }
  }
}
</script>

<style>
.card {
  /* height: 480px; */
  width: 300px;
  margin-top: 5px;
  margin-bottom: 15px;
}

.row:not(:last-child) {
  border-bottom: 1px solid rgb(235 235 235);
  padding-top: 10px;
  margin-bottom: 10px
}

.card-span {
  color: grey;
  font-size: 11px;
}
</style>
