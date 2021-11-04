/*
    建立链接
*/
const taos = require('../../../../src/connector/nodejs/');
var conn = taos.connect({ host: "localhost", user: "root", password: "taosdata", config: "/etc/taos", port: 6030 })
var c1 = conn.cursor();

function checkData(data, row, col, expect) {
    let checkdata = data[row][col];
    if (checkdata == expect) {
        console.log('check pass')
    }
    else {
        console.log('check failed, expect ' + expect + ', but is ' + checkdata)
    }
}



try {
    c1.execute("drop database if exists unsignedtype")
}
catch (err) {
    conn.close();
    throw err;
}
try {
    c1.execute("create database unsignedtype")
} catch (err) {
    conn.close();
    throw err;
}
try{
    c1.execute("use unsignedtype")

}catch(err){
    conn.close();
    throw err;
}
try{
    c1.execute("create stable stb_ut (ts timestamp,ut tinyint unsigned,us smallint unsigned,ui int unsigned,ub bigint unsigned) tags(utt tinyint unsigned,ust smallint unsigned,uit int unsigned,ubt bigint unsigned)")

}catch(err){
    conn.close();
    throw err;
}
//创建子表，tags中的无符号数均为null
try{
    c1.execute("create table ut0 using stb_ut tags (null,null,null,null)")
    c1.execute("select utt,ust,uit,ubt from ut0")
    data = c1.fetchall();
    console.log(data)
    checkData(data,0,0,null)
    checkData(data,0,1,null)
    checkData(data,0,2,null)
    checkData(data,0,3,null)
}catch(err){
    conn.close();
    throw err;
}

//创建子表，tags中的无符号数均为上边界（254，65534，4294967294，18446744073709551614
try{    
    c1.execute("create table ut1 using stb_ut tags (254,65534,4294967294,18446744073709551614)")
    c1.execute("select utt,ust,uit,ubt from ut1")
    data = c1.fetchall();
    console.log(data)
    checkData(data,0,0,254)
    checkData(data,0,1,65534)
    checkData(data,0,2,4294967294)
    checkData(data,0,3,18446744073709551614n)
}catch(err){
    conn.close();
    throw err;
}

//创建子表，tags中的无符号数为0
try{    
    c1.execute("create table ut2 using stb_ut tags (0,0,0,0)")
    c1.execute("select utt,ust,uit,ubt from ut2")
    data = c1.fetchall();
    console.log(data)
    checkData(data,0,0,0)
    checkData(data,0,1,0)
    checkData(data,0,2,0)
    checkData(data,0,3,0)
}catch(err){
    conn.close();
    throw err;
}

//创建子表，tags中的无符号数为100
try{    
    c1.execute("create table ut3 using stb_ut tags (100,100,100,100)")
    c1.execute("select utt,ust,uit,ubt from ut3")
    data = c1.fetchall();
    console.log(data)
    checkData(data,0,0,100)
    checkData(data,0,1,100)
    checkData(data,0,2,100)
    checkData(data,0,3,100)
}catch(err){
    conn.close();
    throw err;
}

//添加数据，所有值均为null
try{
    c1.execute("insert into ut0 values(now,null,null,null,null)")
    c1.execute("select * from ut0")
    data = c1.fetchall();
    console.log(data)
    checkData(data,0,1,null)
    checkData(data,0,2,null)
    checkData(data,0,3,null)
    checkData(data,0,4,null)
}catch(err){
    conn.close();
    throw err;
}

//添加数据，所有值均为0
try{
    c1.execute("insert into ut1 values(now+1s,0,0,0,0)")
    c1.execute("select * from ut1")
    data = c1.fetchall();
    console.log(data)
    checkData(data,0,1,0)
    checkData(data,0,2,0)
    checkData(data,0,3,0)
    checkData(data,0,4,0)
}catch(err){
    conn.close();
    throw err;
}
//添加数据，所有值均为上边界254,65534,4294967294,18446744073709551614
try{
    c1.execute("insert into ut2 values(now+1s,254,65534,4294967294,18446744073709551614)")
    c1.execute("select * from ut2")
    data = c1.fetchall();
    console.log(data)
    checkData(data,0,1,254)
    checkData(data,0,2,65534)
    checkData(data,0,3,4294967294)
    checkData(data,0,4,18446744073709551614n)
}catch(err){
    conn.close();
    throw err;
}
//添加数据，所有值均为上边界254,65534,4294967294,18446744073709551614
try{
    c1.execute("insert into ut3 values(now+1s,100,100,100,100)")
    c1.execute("select * from ut3")
    data = c1.fetchall();
    console.log(data)
    checkData(data,0,1,100)
    checkData(data,0,2,100)
    checkData(data,0,3,100)
    checkData(data,0,4,100)
}catch(err){
    conn.close();
    throw err;
}
conn.close();