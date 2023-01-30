<template>
    <a-card class="card" :title="this.title">
        <div class="row" v-for="(item, key) in data" :key="key">
            <strong class="card-row-title">{{ item.title }}</strong>
            <br>
            <component :is="componentByType(item.type)" :percent="item.value" status="normal" size="small" class="card-span">{{ item.value }}</component>
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
  height: 400px;
  width: 300px;
  margin-top: 5px;
  margin-bottom: 15px;
  margin-right: 15px;
  float: left;
  overflow: hidden;
}

.row:not(:last-child) {
  padding-bottom: 6px;
  border-bottom: 1px solid rgb(235 235 235);
}

.card-row-title {
  font-size: 12px;
}

.card-span {
  color: grey;
  font-size: 11px;
}
</style>
