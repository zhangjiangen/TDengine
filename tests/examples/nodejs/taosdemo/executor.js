const taos = require('td2.0-connector')
const AsyncPool = require('./asyncPool')

class Executor {
  constructor() {
    this.numOfThread = null;
    this.host = null;
    this.port = null;
    this.user = null;
    this.password = null;
    this.config = null
  }

  /**
   * return tdengine's connection
   * @returns {*}
   */
  getConn() {
    return taos.connect({
      host: this.host,
      user: this.port,
      password: this.password,
      config: this.config,
      port: this.port
    })
  }

  /**
   * execute sql like create db,drop db. with single thread
   * @param inputSql
   */
  execute(inputSql) {
    let conn = getConn()
    conn.cursor().execute(inputSql)
    conn.close()
  }

  /**
   * execute multiple sql with fixed number of threads
   * @param sqlArray
   */
  executeAsyncPool(sqlArray) {
    let conn = getConn()
    const iteratorFn = sql => {
      new Promise(resolve => {
        conn.cursor().execute(sql)
        return resolve()
      })
    }
    //calling  the async pool to execute the sql
    asyncPool.AsyncPool(this.numOfThread, sqlArray, iteratorFn).then(conn.close()).catch(e => console.log(e))
  }

}

module.exports = Executor