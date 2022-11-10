[toc]

## 前言

水文-ts中对中文进行base64编码

完整代码见仓库。

## ts环境准备

参考：[如何建立一个新的 TypeScript 项目](https://www.gingerdoc.com/tutorials/typescript-new-project)

```shell
# npm install -g ts-node
npm install -g typescript
tsc.cmd --init
```

我之前安装过nodejs环境：[fetch的简单使用](https://blog.csdn.net/sinat_38816924/article/details/126684293)。所以不再需要安装nodejs。

我们使用`npm`安装[typescript](https://www.npmjs.com/package/typescript)。关于npm的使用，可以参考[npm 模块安装机制简介](https://www.ruanyifeng.com/blog/2016/01/npm-install.html)。至于typescript是一种用于应用程序级 JavaScript 的语言，可以将ts编写的程序编译成js。安装typescript的时候，会包含[tsc](https://www.npmjs.com/package/tsc)。

注：上面参考链接中，使用了`npx`,它可以调用项目内部模块，避免全局安装，可以参考：[npx 使用教程](https://www.ruanyifeng.com/blog/2019/02/npx.html)。

接着，我们使用`tsc.cmd --init`,创建一个[tsconfig.json](https://www.tslang.cn/docs/handbook/tsconfig-json.html)。tsconfig.json文件中指定了用来编译这个项目的根文件和编译选项。

---

## ts中对utf8字符串进行base64编码

参考：[Using Javascript's atob to decode base64 doesn't properly decode utf-8 strings](https://stackoverflow.com/questions/30106476/using-javascripts-atob-to-decode-base64-doesnt-properly-decode-utf-8-strings)

使用[js-base64](https://www.npmjs.com/package/js-base64)对utf8字符串进行base64编码。

```shell
npm install --save js-base64
# 安装package.json中的依赖
# npm install
# npm uninstall typescript
```

我用demo测了下，应该能用。

```typescript
import { Base64 } from 'js-base64'

console.debug("hello world")

const str = "中文"

// Base64
console.debug(Base64.encode(str))
console.debug(Base64.decode(Base64.encode(str)))
```

先编译，`tsc.cmd`命令按照`tscconfig.json`中的配置进行编译，生成js。`node.exe .\dist\index.js`,使用`node`命令运行js程序。输出如下。

```shell
hello world
5Lit5paH
中文
```