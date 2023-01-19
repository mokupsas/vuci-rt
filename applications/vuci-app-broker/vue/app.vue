<template>
    <div class="test">
        <p>test this</p>
    </div>
</template>

<script>
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