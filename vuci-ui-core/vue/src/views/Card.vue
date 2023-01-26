<template>
    <a-card :title="this.title" style="width: 300px">
      <a slot="extra" href="#">more</a>

        <div class="row" v-for="(item, key) in data" :key="key">

          <!-- Row without columns -->
          <div v-if="!getColumns(item)">
            <strong>{{ item.name }}</strong>
            <br>
            <span>{{ item.data }}</span>
          </div>

          <!-- Row with columns -->
          <div v-if="getColumns(item)">
            <strong>{{ item.name }}</strong>
            <a-row>
              <a-col :span="12" v-for="(item, key) in data" :key="key">
                <strong>{{ item.name }}</strong>
                <br>
                <span>{{ item.data }}</span>
              </a-col>
            </a-row>
          </div>

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
    getColumns (item) {
      if (item.columns && item.columns > 1) { return item.columns }
      return false
    }
  },
  created () {
    console.log(this.getColumns(this.data[0]))
  }
}
</script>

<style>
.row:not(:last-child) {
  border-bottom: 1px solid rgb(235 235 235);
  padding-top: 10px;
  margin-bottom: 10px
}
</style>
