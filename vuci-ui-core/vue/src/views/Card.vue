<template>
    <a-card class="card" :title="this.title">
        <div class="row" v-for="(item, key) in data" :key="key">

            <strong>{{ item.name }}</strong>
            <br>
            <span v-if="!item.progress">{{ item.data }}</span>
            <a-progress v-if="item.progress" :percent="item.data"/>

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
  }
}
</script>

<style>
.card {
  width: 300px;
  margin-top: 5px;
  margin-bottom: 15px;
}

.row:not(:last-child) {
  border-bottom: 1px solid rgb(235 235 235);
  padding-top: 10px;
  margin-bottom: 10px
}
</style>
