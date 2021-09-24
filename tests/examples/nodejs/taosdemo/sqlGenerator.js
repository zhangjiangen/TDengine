const UtileTool = require('./utilTool')

class SqlGenerator {
  createDB(db, replica) {
    let createDB = "create database if not exists " + db + " replica " + replica + " keep 36500 ;";
    return createDB;
  }

  dropDB(db) {
    let dropDB = "drop database if  exists " + db + ";"
    return dropDB
  }

  createNtable(db,prefix,numOfTable) {
    let createArr = []
    // create normal table
    for (let i = 0; i < numOfTable; i++) {
      let ntable = "CREATE TABLE IF NOT EXISTS " + db + "." + prefix + "_" + i + " (TS TIMESTAMP,CURRENT FLOAT,VOLTAGE INT ,PHASE FLOAT );"
      createArr.push(ntable)
      return createArr
    }
  }

  createStable(db,table) {
    let createArr = []
    //CREATE STABLE meters (ts timestamp, current float, voltage int, phase float) TAGS (location binary(64), groupId int);
    let stable = "CREATE STABLE " + this.db + "."+table+" (ts timestamp, current float, voltage int, phase float) TAGS (location binary(64), groupId int);"
    createArr.push(stable)
    return createArr
  }

  insertStable(db,tableName,stableName,record,tag1,tag2,ts,interval) {
    let insertArr =[]
    let sql =""
    // //INSERT INTO d1001 USING meters TAGS ("Beijng.Chaoyang", 2) VALUES (now, 10.2, 219, 0.32);
    for(let i = 0;i<record;++i){
      sql = "INSERT INTO "+ db+"."+tableName+ " USING "+stableName+" TAGS(" + tag1 +","+ tag2+") VALUES( "+(ts+interval)+","+UtileTool.randomInt()+","+UtileTool.randomFloat()+");"
      insertArr.push(sql)
    }
    return insertArr
  }

  insertNormalTable(db,prefix,record,ts,interval){
    let insertArr = []
    let sql = ""
    for(let i = 0; i < record ;i++){
      sql = "INSERT INTO " + db+"."+prefix + " VALUES(" + ts+interval + ","+UtileTool.randomInt()+","+UtileTool.randomFloat()+");"
      insertArr.push(sql)
    }
    return insertArr
  }

  selectSql() {
    let arr = []
    return arr
  }

}
module.exports = SqlGenerator