import { fileURLToPath, URL } from 'node:url'

import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import { Server } from 'node:http'

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [vue()],
  resolve: {
    alias: {
      '@': fileURLToPath(new URL('./src', import.meta.url))
    }
  },
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
