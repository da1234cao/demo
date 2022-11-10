import { Base64 } from 'js-base64'

console.debug("hello world")

const str = "中文"

// Base64
console.debug(Base64.encode(str))
console.debug(Base64.decode(Base64.encode(str)))