[TOC]

## 前言

公司的客户端使用了[GitHub - chromiumembedded/cef](https://github.com/chromiumembedded/cef) 。所以，有时候不得不写前端代码。有时候逻辑很清晰，但在语法上折腾半天。

这周在一个[Fetch API - MDN](https://developer.mozilla.org/zh-CN/docs/Web/API/Fetch_API) 调用上踩了坑，所以去读了它的文档，顺道整理下。

在开始之前，最好和[vue](https://cn.vuejs.org/)混个脸熟，知道它是干啥的。简单的刷下视频快速入门下：[2022年最新版Vue3全套教程](https://www.bilibili.com/video/BV1QA4y1d7xf?p=70)

## vue开发环境搭建

参考下：[Windows 环境搭建 Vue 开发环境 | 小决的专栏](https://jueee.github.io/2020/10/2020-10-31-Windows%E7%8E%AF%E5%A2%83%E6%90%AD%E5%BB%BAVue%E5%BC%80%E5%8F%91%E7%8E%AF%E5%A2%83/)、[十分钟上手-搭建vue2.0开发环境](https://www.jianshu.com/p/0c6678671635)

1. 安装[Node.js](https://nodejs.org/zh-cn/)。安装之后，验证安装成功。
   
   ```shell
   node.exe -v
   v16.17.0
   ```

2. 全局安装[vue-cli](https://cli.vuejs.org/zh/index.html)脚手架 - 略
   
   ```shell
    npm.cmd install --global vue-cli
    4 vulnerabilities (2 moderate, 2 high)
    To address all issues (including breaking changes), run:
   
    npm audit fix --force
    npm WARN using --force Recommended protections disabled.
    npm ERR! code ENOLOCK
    npm ERR! audit This command requires an existing lockfile.
    npm ERR! audit Try creating one first with: npm i --package-lock-only
    npm ERR! audit Original error: loadVirtual requires existing shrinkwrap file
   
    npm.cmd remove -g @vue/cli
   ```

3. 得，我们[替换](https://cn.vuejs.org/guide/scaling-up/tooling.html#project-scaffolding)使用[Vite](https://cn.vitejs.dev/)
   
   ```shell
   npm init vue@latest
   
   Need to install the following packages:
   create-vue@3.3.2
   Ok to proceed? (y) y
   
   Vue.js - The Progressive JavaScript Framework
   
   √ Project name: ... fetch-demo
   √ Add TypeScript? ... No / Yes
   √ Add JSX Support? ... No / Yes
   √ Add Vue Router for Single Page Application development? ... No / Yes
   √ Add Pinia for state management? ... No / Yes
   √ Add Vitest for Unit Testing? ... No / Yes
   √ Add Cypress for both Unit and End-to-End testing? ... No / Yes
   √ Add ESLint for code quality? ... No / Yes
   
   Scaffolding project in E:\code\tmp\fetch-demo...
   
   Done. Now run:
   
     cd fetch-demo
     npm install
     npm run dev
   ```

4. vscode安装插件。参考：[VSCode 开发Vue必备插件](https://zhuanlan.zhihu.com/p/347926284)

## fetch demo

[Fetch API - MDN](https://developer.mozilla.org/zh-CN/docs/Web/API/Fetch_API) 提供了一个获取资源的接口（包括跨域请求）。`fetch()` 必须接受一个参数——资源的路径。无论请求成功与否，它都返回一个 [Promise](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference/Global_Objects/Promise) 对象。一个 `Promise` 对象代表一个在这个 promise 被创建出来时不一定已知值的代理。它让你能够把异步操作最终的成功返回值或者失败原因和相应的处理程序关联起来。（类似于[std::promise - cppreference.com](https://zh.cppreference.com/w/cpp/thread/promise)，没用过这个，[std::future - cppreference.com](https://zh.cppreference.com/w/cpp/thread/future)的接口层次高些。）

关于`fetch`的介绍，还可以参考：[Fetch API 教程 - 阮一峰的网络日志](https://www.ruanyifeng.com/blog/2020/12/fetch-tutorial.html)、[promise - How to use fetch in TypeScript - Stack Overflow](https://stackoverflow.com/questions/41103360/how-to-use-fetch-in-typescript)

代码很简单，访问一个地址，并获取响应状态码。(原本是想访问一个页面，然后浏览器展示fetch到的页面，这似乎不容易，至少对于我这个web菜狗，是做不到的)

```vue
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

```

代码运行的时候，会出现跨域请求的问题。

参考：[跨源资源共享（CORS） - HTTP | MDN](https://developer.mozilla.org/zh-CN/docs/Web/HTTP/CORS)、[跨域的解决方法有哪些？JSONP的原理？CORS怎么使用？Nginx如何设置？ - BiliBili](https://www.bilibili.com/video/BV1Ei4y1o7jK/)

跨域请求的原理是，浏览器执行当前页面服务器的请求，而请求的资源却在另一个域名的服务器上。跨域请求，可能出现[CSRF攻击_大1234草的博客-CSDN博客](https://da1234cao.blog.csdn.net/article/details/107124466)的问题。但，必须可以在不同站点间跳来跳去，比如超链接，否则站点会成为一个个孤岛。

`vite`可以通过proxy来解决跨站问题，可参考：[vite-proxy-BiliBili](https://www.bilibili.com/video/BV1QA4y1d7xf?p=69)、[开发服务器选项 | Vite 官方中文文档](https://cn.vitejs.dev/config/server-options.html#server-proxy)

配置如下。

```ts
export default defineConfig({
  xxxx,
  xxxx,
  server: {
    proxy:{
      '/baidu':{
        target:'https://www.baidu.com',
        changeOrigin:true,
        rewrite: path => path.replace(/^\/baidu/,'')
      },
      '/koal':{
        target:'http://www.koal.com',
        changeOrigin:true,
        rewrite: path => path.replace(/^\/koal/,'')
      }
    }
  }
})
```

官方文档中写道，是使用 [`http-proxy`](https://github.com/http-party/node-http-proxy)完成代理。

猜测代理过程大概是这样。浏览器去执行请求，是跨站。那浏览器把锅推回去，让服务器自己去请求其他服务器的资源。请求结束之后，再将内容返回给浏览器就好了。是一个本地的反向代理过程。
