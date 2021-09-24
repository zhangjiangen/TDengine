
function printPretty(str1, str2) {
  let length1 = str1.length
  let resStr = printEmpty(10) + str1 + printEmpty(30 - 10 - length1) + str2
  return resStr
}

function printEmpty(num) {
  let str = ""
  for (let i = 1; i <= num; i++) {
    str += " "
  }
  return str
}

function randomString(length){
  const strPool = "1234567890qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM"
  let str = ""
  for (let i=0;i<length;i++){
   str +=strPool.charAt(Math.ceil(Math.random()*strPool.length))
  }
 return str
}
function randomInt(){
  return Math.ceil(Math.random()*100)
}
function randomDouble(){
  return Math.ceil(Math.random()*10000)
}

function randomFloat(){
  return Math.ceil(Math.random()*100)
}
function printArr(array){
  array.forEach(element=>{console.log(element)})
}
module.exports = {printEmpty,printPretty,randomString,randomDouble,randomInt,randomFloat,printArr}