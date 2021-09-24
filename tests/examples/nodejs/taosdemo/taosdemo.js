//the entry of taosdemo
const ReadArgument = require('./readArgument')
const SqlGenerator = require('./sqlGenerator')
const UtilTool = require('./utilTool')
//const Executor = require('./executor')

//readArugment
ReadArgument.readArgument()
//sql generator
const sqlGen = new SqlGenerator()
let createDB = sqlGen.createDB('taosdemo',1)
let dropDB = sqlGen.dropDB('taosdemo')

let createTableArr = sqlGen.createStable('taosdemo','stable')
let createStableArr = sqlGen.createNtable('taosdemo','d',10)

let insertNtable = sqlGen.insertNormalTable('taosdemo','d_0',10,100,1)
let insertStable = sqlGen.insertStable('taosdata','d_0','meters',10,1,'fg',100,1)//,UtilTool.randomInt(),UtilTool.randomString(64)

console.log(createDB)
console.log("=================")
console.log(dropDB)
UtilTool.printArr(createTableArr)
console.log("=================")
UtilTool.printArr(createStableArr)
console.log("================")
UtilTool.printArr(insertNtable)
console.log("================")
UtilTool.printArr(insertStable)

  //create db
  //create table
  //insert
  //select



