<script lang="ts">
export default {
  data() {
      return {
        url: String,
        page: String
      }
    },
  methods: {
    async copy_page(url:string) : Promise<any> {
      this.url = url
      this.page = await fetch(this.url,{
        method: 'get',
        headers: {
          'Content-Type': 'application/json',
        },
      }).then(resp =>{
        console.debug(resp)
        return resp.status
      })
    }, 
  },
  mounted() {
    this.page = this.copy_page('/baidu/s?wd=fetch-demo')
    console.log('load app vue')
  }
}
</script>

<template>
  <div>{{page}}</div>
</template>

<style scoped>
</style>
