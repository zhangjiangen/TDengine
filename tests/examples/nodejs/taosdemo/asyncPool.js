/**
 *
 * @param poolLimit concurrency limitation
 * @param array   promise array (in this situation is the the sql need to be execute)
 * @param iteratorFunction callback
 */
function AsyncPool(poolLimit, array, iteratorFunction) {
  let i = 0;
  const ret = [];
  const executing = [];
  const enqueue = function() {
    if (i === array.length) {
      return Promise.resolve();
    }
    const item = array[i++];
    const p = Promise.resolve().then(() => iteratorFunction(item, array));
    ret.push(p);

    let r = Promise.resolve();

    if (poolLimit <= array.length) {
      const e = p.then(() => executing.splice(executing.indexOf(e), 1));
      executing.push(e);
      if (executing.length >= poolLimit) {
        r = Promise.race(executing);
      }
    }

    return r.then(() => enqueue());
  };
  return enqueue().then(() => Promise.all(ret));
}
module.exports = {AsyncPool}