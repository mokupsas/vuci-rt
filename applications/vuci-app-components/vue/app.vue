<template>
    <div class="example">
        <a-input v-model="time"></a-input>
        <a-button type="primary" @click="get_time">
            Get time here
        </a-button>
    </div>
</template>

<script>
import bla from './components/'
export default {
    data() {
        return {
            time: ''
        }
    },
    methods: {
        get_time() {
            this.$rpc.call('example', 'get_time', {}).then(r => {
                this.time = r.time
            })
        }
    }
}
</script>

<style></style>