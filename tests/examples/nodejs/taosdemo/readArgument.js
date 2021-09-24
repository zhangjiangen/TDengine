const UtilTool = require('./utilTool')
//argument name mapping
const argNameMapping = {
  f: 'file',
  u: 'user',
  p: 'password',
  c: 'cfgDir',
  h: 'host',
  P: 'port',
  I: 'interface',
  d: 'database',
  a: 'replica',
  m: 'tablePrefix',
  s: 'sqlFile',
  N: 'normalTable',
  o: 'outFile',
  q: 'queryMode',
  b: 'dataType',
  w: 'width',
  T: 'numOfThread',
  i: 'insertInterval',
  S: 'timestampInterval',
  B: 'interlaceRow',
  r: 'recordPerReq',
  t: 'numOfTable',
  n: 'numOfRecord',
  x: 'noInsertFlag',
  y: 'answer',
  O: 'disorderRatio',
  R: 'disorderRange',
  g: 'debugFlag',
  V: 'version',
  M: 'random',
  l: 'numOfColumn'
}
//argument default value
const argValue = {
  file: './meta.jason',
  user: 'root',
  password: 'taosdata',
  cfgDir: '/ect/taos',
  host: '127.0.0.1',
  port: 0,
  interface: 'taosc',
  database: 'test',
  replica: 1,
  tablePrefix: 'd',
  sqlFile: null,
  normalTable: false,
  outFile: './output.txt',
  queryMode: 0,
  dataType: 'default FLOAT,INT,FLOAT',
  width: 64,
  numOfThread: 10,
  insertInterval: 0,
  timestampInterval: 1,
  interlaceRow: 0,
  recordPerReq: 30000,
  numOfTable: 10000,
  numOfRecord: 10000,
  random: false,
  noInsertFlag: false,
  answer: 'yes',
  disorderRatio: 0,
  disorderRange: 0,
  debugFlag: false,
  usage: "Give short usage message",
  version: "1.0.0.0",
  numOfColumn: 1,
}

/**
 *
 * @param argTag the name of argument like h
 * @returns {*}
 */
function getArgumentName(argTag) {
  return argNameMapping[argTag]
}

/**
 * Read user's input, and set the corresponding argument value
 * @param argTag user's input argument tag
 * @param value user's input values
 */
function setArgumentValue(argTag, value) {
  argValue[getArgumentName(argTag)] = value
}

/**
 * return the value of the given argument name
 * @param argName the name of the argument,like "host,numOfThread"
 * @returns {*}
 */
function getArgumentValue(argName) {
  return argValue[argName]
}

/**
 * return true if the incoming argument is a valid argument
 * @param argTag
 * @returns {boolean}
 */
function isArgument(argTag) {
  for (let arg in argNameMapping) {
    if (arg === argTag) {
      return true
    }
  }
  return false
}


function printHelp() {
  console.log(UtilTool.printEmpty(10) + 'Usage: taosdemo [OPTION...]')
  console.log(UtilTool.printEmpty(10) + '--help show usage')
  console.log(UtilTool.printPretty('-f <FILE>', 'The meta file to the execution procedure. Default is ./meta.json.'))
  console.log(UtilTool.printPretty('-u <USER>', 'The user name to use when connecting to the server.'))
  console.log(UtilTool.printPretty('-p <PASSWORD>', 'The password to use when connecting to the server.'))
  console.log(UtilTool.printPretty('-c <CONFIG_DIR>', 'Configuration directory.'))
  console.log(UtilTool.printPretty('-h <HOST>', 'TDengine server FQDN to connect,default localhost.'))
  console.log(UtilTool.printPretty('-P <PORT>', 'The TCP/IP port number to use for the connection.'))
  console.log(UtilTool.printPretty('-I <INTERFACE>', 'The interface (taosc, rest, and stmt) taosdemo uses,default taosc'))
  console.log(UtilTool.printPretty('-d <DATABASE>', 'TDestination database,default is \'test\'.'))
  console.log(UtilTool.printPretty('-a <REPLICA>', 'Set the replica parameters of the database, default 1, min: 1, max: 3.'))
  console.log(UtilTool.printPretty('-m <TABLE-PREFIX>', 'Table prefix name,default is \'d\'.'))
  console.log(UtilTool.printPretty('-s <SQL-FILE>', 'The select sql file.'))
  console.log(UtilTool.printPretty('-N <NORMAL-TABLE>', 'Use normal table flag.'))
  console.log(UtilTool.printPretty('-o <OUTPUT-FILE>', 'Direct output to the named file,default is \'./output.txt\'.'))
  console.log(UtilTool.printPretty('-q <QUERY_MODE>', 'Query mode -- 0: SYNC, 1: ASYNC. Default is SYNC.'))
  console.log(UtilTool.printPretty('-b <DATATYPE>', 'The data_type of columns, default: FLOAT, INT, FLOAT.'))
  console.log(UtilTool.printPretty('-w <WIDTH>', 'The width of data_type \'BINARY\' or \'NCHAR\'. Default is 64'))
  console.log(UtilTool.printPretty('-T <THREADS>', 'he number of threads,default is 10.'))
  console.log(UtilTool.printPretty('-i <INTERVAL>', 'The sleep time (ms) between insertion,default is 0.'))
  console.log(UtilTool.printPretty('-S <TIME_STEP>', 'The timestamp step between insertion,default is 1.'))
  console.log(UtilTool.printPretty('-B <INTERLACE-ROWS>', 'The interlace rows of insertion,default is 0.'))
  console.log(UtilTool.printPretty('-r <REC-PER-REQ>', 'The number of records per request,default is 30000.'))
  console.log(UtilTool.printPretty('-t <TABLES>', 'The number of records per request,default is 30000.'))
  console.log(UtilTool.printPretty('-n <RECORDS>', 'The number of records per table. Default is 10000.'))
  console.log(UtilTool.printPretty('-x <NO-INSERT>', 'No-insert flag.'))
  console.log(UtilTool.printPretty('-y <ANSWER>', 'No-insert flag.'))
  console.log(UtilTool.printPretty('-O <DIS-RATIO>', 'Insert order mode--0: In order, 1 ~ 50: disorder ratio. Default is in order.'))
  console.log(UtilTool.printPretty('-R <DIS-RANGE>', 'Disorder data\'s range, ms, default is 1000.'))
  console.log(UtilTool.printPretty('-g <DEBUG>', 'Print debug info.'))
  console.log(UtilTool.printPretty('-? <HELP>', 'Give this help list.'))
  console.log(UtilTool.printPretty('-V <VERSION>', 'Print program version.'))
  console.log(UtilTool.printPretty('-M <RANDOM>', 'The value of records generated are totally random.The default is to simulate power equipment scenario.'))
  console.log(UtilTool.printPretty('-l <COLUMNS>', 'The number of columns per record. Demo mode by default is 1 (float, int, float). Max values is 4095'))
  console.log(UtilTool.printPretty("", 'All of the new column(s) type is INT. If use -b to specify column type, -l will be ignored.'))
}

function printUsage() {
  console.log(" Usage: taosdemo [-f JSONFILE] [-u USER] [-p PASSWORD] [-c CONFIG_DIR]\n" +
    "                    [-h HOST] [-P PORT] [-I INTERFACE] [-d DATABASE] [-a REPLICA]\n" +
    "                    [-m TABLEPREFIX] [-s SQLFILE] [-N] [-o OUTPUTFILE] [-q QUERYMODE]\n" +
    "                    [-b DATATYPES] [-w WIDTH_OF_BINARY] [-l COLUMNS] [-T THREADNUMBER]\n" +
    "                    [-i SLEEPTIME] [-S TIME_STEP] [-B INTERLACE_ROWS] [-t TABLES]\n" +
    "                    [-n RECORDS] [-M] [-x] [-y] [-O ORDERMODE] [-R RANGE] [-a REPLIcA][-g]\n" +
    "                    [--help] [--usage] [--version]")
}

function printTaskInfo() {

}

function readArgument() {
  if (global.process.argv.length == 2) {
    console.log("Run taosdemo using default configuration")
  } else {
    for (let i = 2; i < global.process.argv.length; i += 2) {
      let key = global.process.argv[i].slice(1);
      let value = global.process.argv[i + 1];
      if (key == '-help') {
        printHelp()
        return;
      } else if (!isArgument(key)) {
        console.log("-" + key + " is an unknown argument")
        printUsage()
        return;
      } else if(isArgument(key)&&(value === undefined)){
        console.log( console.log("-" + key + "'s value is undefined"))
        return ;
      }else{
        setArgumentValue(key, value)
      }
    }
  }
  console.log(argValue)
  //before execute need to print the task information
}


module.exports = {readArgument, getArgumentValue}