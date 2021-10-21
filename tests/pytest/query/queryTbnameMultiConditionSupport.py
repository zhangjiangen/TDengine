###################################################################
#           Copyright (t) 2016 by TAOS Technologies, Inc.
#                     All rights reserved.
#
#  This file is proprietary and confidential to TAOS Technologies.
#  No part of this file may be reproduced, stored, transmitted,
#  disclosed or used in any form or by any means other than as
#  expressly provided by the written permission from Jianhui Tao
#
###################################################################

# -*- coding: utf-8 -*-
from util.log import tdLog
from util.cases import tdCases
from util.sql import tdSql
from util.common import tdCom
class TDTestCase:
    def init(self, conn, logSql):
        tdLog.debug("start to execute %s" % __file__)
        tdSql.init(conn.cursor(), logSql)

    def insertData(self, tb_name):
        insert_sql_list = [f'insert into {tb_name} values ("2021-01-01 12:00:00", 1, 1, 1, 3, 1.1, 1.1, "binary", "nchar", true, 1)',
                           f'insert into {tb_name} values ("2021-01-05 12:00:00", 2, 2, 1, 3, 1.1, 1.1, "binary", "nchar", true, 2)',
                           f'insert into {tb_name} values ("2021-01-07 12:00:00", 1, 3, 1, 2, 1.1, 1.1, "binary", "nchar", true, 3)',
                           f'insert into {tb_name} values ("2021-01-09 12:00:00", 1, 2, 4, 3, 1.1, 1.1, "binary", "nchar", true, 4)',
                           f'insert into {tb_name} values ("2021-01-11 12:00:00", 1, 2, 5, 5, 1.1, 1.1, "binary", "nchar", true, 5)',
                           f'insert into {tb_name} values ("2021-01-13 12:00:00", 1, 2, 1, 3, 6.6, 1.1, "binary", "nchar", true, 6)',
                           f'insert into {tb_name} values ("2021-01-15 12:00:00", 1, 2, 1, 3, 1.1, 7.7, "binary", "nchar", true, 7)',
                           f'insert into {tb_name} values ("2021-01-17 12:00:00", 1, 2, 1, 3, 1.1, 1.1, "binary8", "nchar", true, 8)',
                           f'insert into {tb_name} values ("2021-01-19 12:00:00", 1, 2, 1, 3, 1.1, 1.1, "binary", "nchar9", true, 9)',
                           f'insert into {tb_name} values ("2021-01-21 12:00:00", 1, 2, 1, 3, 1.1, 1.1, "binary", "nchar", false, 10)',
                           f'insert into {tb_name} values ("2021-01-23 12:00:00", 1, 3, 1, 3, 1.1, 1.1, Null, Null, false, 11)'
                           ]
        for sql in insert_sql_list:
            tdSql.execute(sql)

    def initTb(self):
        tdCom.cleanTb()
        tb_name = tdCom.getLongName(8, "letters")
        tdSql.execute(
            f"CREATE TABLE {tb_name} (ts timestamp, c1 tinyint, c2 smallint, c3 int, c4 bigint, c5 float, c6 double, c7 binary(100), c8 nchar(200), c9 bool, c10 int)")
        self.insertData(tb_name)
        return tb_name

    def initStb(self):
        tdCom.cleanTb()
        tb_name = tdCom.getLongName(8, "letters")
        tdSql.execute(
            f"CREATE TABLE {tb_name} (ts timestamp, c1 tinyint, c2 smallint, c3 int, c4 bigint, c5 float, c6 double, c7 binary(100), c8 nchar(200), c9 bool, c10 int) tags (t1 tinyint, t2 smallint, t3 int, t4 bigint, t5 float, t6 double, t7 binary(100), t8 nchar(200), t9 bool, t10 int)")
        for i in range(1, 6):    
            tdSql.execute(
                f'CREATE TABLE {tb_name}_sub_{i} using {tb_name} tags ({i}, {i}, {i}, {i}, {i}.{i}, {i}.{i}, "binary{i}", "nchar{i}", true, {i})')
            self.insertData(f'{tb_name}_sub_{i}')
        return tb_name

    def checkTbColTypeCondition(self):
        '''
            Ordinary table full column type Condition
        '''
        tb_name = self.initTb()
        ## ts > and <=
        query_sql = f'select tbname,ts from {tb_name} where ts > "2021-01-11 12:00:00" and ts <= "2021-01-13 12:00:00"'
        tdSql.query(query_sql)
        tdSql.checkRows(1)
        tdSql.checkData(0, 1, "2021-01-13 12:00:00.000")

        ## int >= or <
        query_sql = f'select tbname,c3 from {tb_name} where c3 >= 5 or c3 < 2'
        tdSql.query(query_sql)
        tdSql.checkRows(10)

        ## tiny int != and <> 
        query_sql = f'select tbname,c1 from {tb_name} where c1 <> 1 and c1 != 3'
        tdSql.query(query_sql)
        tdSql.checkRows(1)
        tdSql.checkData(0, 1, 2)

        ## binary is Null or =
        query_sql = f'select tbname,c7 from {tb_name} where c7 = "binary8" or c7 is Null'
        tdSql.query(query_sql)
        tdSql.checkRows(2)
        tdSql.checkData(0, 1, "binary8")
        tdSql.checkData(1, 1, None)

        ## bool =false or =true
        query_sql = f'select tbname,c9 from {tb_name} where c9 = true or c9 = false'
        tdSql.query(query_sql)
        tdSql.checkRows(11)

        ## float = and nchar is not Null
        query_sql = f'select tbname,c5,c8 from {tb_name} where c5 = 6.6 and c8 is not Null'
        tdSql.query(query_sql)
        tdSql.checkRows(1)

        ## double != or bigint > or smallint =
        query_sql = f'select tbname,c6,c4 from {tb_name} where c6 != "1.1" or c4 >4 or c2=3'
        tdSql.query(query_sql)
        tdSql.checkRows(4)

        ## all types or
        query_sql = f'select tbname,ts,c1,c2,c3,c4,c5,c6,c7,c8,c9 from {tb_name} where c1=2 or c2=3 or c3=4 or c4=5 or c5=6.6 or c6=7.7 or c7="binary8" or c8="nchar9" or c9=false'
        tdSql.query(query_sql)
        tdSql.checkRows(10)
    
    def queryTbnameCondition(self):
        tb_name = self.initStb()
        ## tbname > and <
        query_sql = f'select tbname from {tb_name} where tbname > "{tb_name}_sub_2" and tbname < "{tb_name}_sub_4"'
        tdSql.query(query_sql)
        tdSql.checkRows(1)
        tdSql.checkData(0, 0, f"{tb_name}_sub_3")

        ## tbname >= and <=
        query_sql = f'select tbname from {tb_name} where tbname >= "{tb_name}_sub_2" and tbname <= "{tb_name}_sub_4"'
        tdSql.query(query_sql)
        tdSql.checkRows(3)

        ## tbname != and is not Null
        query_sql = f'select tbname from {tb_name} where tbname != "{tb_name}_sub_2" and tbname is not Null'
        tdSql.query(query_sql)
        tdSql.checkRows(4)

        ## tbname <> or is Null
        query_sql = f'select tbname from {tb_name} where tbname <> "{tb_name}_sub_2" or tbname is Null'
        tdSql.query(query_sql)
        tdSql.checkRows(4)

        ## between and and
        query_sql = f'select tbname from {tb_name} where tbname between "{tb_name}_sub_2" and "{tb_name}_sub_4" and tbname != "{tb_name}_sub_3"'
        tdSql.query(query_sql)
        tdSql.checkRows(2)

        ## match or like
        query_sql = f'select tbname from {tb_name} where tbname match "{tb_name}_sub_[34]" or tbname like "{tb_name}%5"'
        tdSql.query(query_sql)
        tdSql.checkRows(3)

        ## nmatch or in
        query_sql = f'select tbname from {tb_name} where tbname nmatch "{tb_name}_sub_[34]" or tbname in ("{tb_name}_sub_3", "{tb_name}_sub_2")'
        tdSql.query(query_sql)
        tdSql.checkRows(4)

    def checkStbTagTypeCondition(self):
        '''
            Super table full type Condition
        '''
        tb_name = self.initStb()
        ## ts > and <=
        query_sql = f'select tbname,ts from {tb_name} where ts > "2021-01-11 12:00:00" and ts <= "2021-01-13 12:00:00"'
        tdSql.query(query_sql)
        tdSql.checkRows(5)
        tdSql.checkData(0, 1, "2021-01-13 12:00:00.000")

        ## int >= or <
        query_sql = f'select tbname,t3 from {tb_name} where t3 >= 5 or t3 < 2'
        tdSql.query(query_sql)
        tdSql.checkRows(2)

        ## tiny int != and <> 
        query_sql = f'select tbname,t1 from {tb_name} where t1 <> 1 and t1 != 3'
        tdSql.query(query_sql)
        tdSql.checkRows(3)
        tdSql.checkData(0, 1, 2)
        tdSql.checkData(2, 1, 5)
        
        ## binary is Null or =
        query_sql = f'select tbname,t7 from {tb_name} where t7 = "binary5" or t7 is Null'
        tdSql.query(query_sql)
        tdSql.checkRows(1)
        tdSql.checkData(0, 1, "binary5")

        ## bool =false or =true
        query_sql = f'select tbname,t9 from {tb_name} where t9 = false'
        tdSql.query(query_sql)
        tdSql.checkRows(0)

        ## float = and nchar is not Null
        query_sql = f'select tbname,t5,t8 from {tb_name} where t5 = 3.3 and t8 = "nchar3"'
        tdSql.query(query_sql)
        tdSql.checkRows(1)

        ## double != and bigint >
        query_sql = f'select tbname,t6,t4 from {tb_name} where t6 != "1.1" or t4 >4'
        tdSql.query(query_sql)
        tdSql.checkRows(4)

        ## smallint = or tbname =
        query_sql = f'select tbname,t2 from {tb_name} where t2 = 2 or tbname="{tb_name}_sub_5"'
        tdSql.query(query_sql)
        tdSql.checkRows(2)
        tdSql.checkData(1, 0, f'{tb_name}_sub_5')

        ## smallint = or tbname like
        query_sql = f'select tbname,t2 from {tb_name} where t2 = 2 or tbname like "%_sub_5"'
        tdSql.query(query_sql)
        tdSql.checkRows(2)
        tdSql.checkData(1, 0, f'{tb_name}_sub_5')

        ## smallint = or tbname in
        query_sql = f'select tbname,t2 from {tb_name} where t2 = 2 or tbname in ("{tb_name}_sub_3", "{tb_name}_sub_1")'
        tdSql.query(query_sql)
        tdSql.checkRows(3)
        tdSql.checkData(2, 0, f'{tb_name}_sub_3')

        ## all types or
        query_sql = f'select tbname,t1,t2,t3,t4,t5,t6,t7,t8,t9 from {tb_name} where t1=1 or t2=1 or t3=1 or t4=2 or t5=2.2 or t6=7.7 or t7="binary8" or t8="nchar9" or t9=false or tbname in ("{tb_name}_sub_5", "{tb_name}_sub_6") or tbname like "%sub_4"'
        tdSql.query(query_sql)
        tdSql.checkRows(4)
        tdSql.checkData(3, 0, f'{tb_name}_sub_5')

        ## with cols' all types
        query_sql = f'select tbname,c1,c2,c3,c4,c5,c6,c7,c8,c9,c10,t1,t2,t3,t4,t5,t6,t7,t8,t9 from {tb_name} where (c1=2 and c2=3 and c3=4 and c4=5 and c5=6.6 and c6=7.7 or c7="binary8" or c8="nchar9" and c9=false) and (t1=1 or t2=1 or t3=1 or t4=2 or t5=2.2 or t6=7.7 or t7="binary8" or t8="nchar9" or t9=false or tbname in ("{tb_name}_sub_5", "{tb_name}_sub_6") or tbname like "%sub_4") and tbname match "{tb_name}_sub_[45]" and tbname nmatch "{tb_name}_sub_[4]"'
        tdSql.query(query_sql)
        tdSql.checkRows(1)
        tdSql.checkData(0, 0, f'{tb_name}_sub_5')

    def run(self):
        tdSql.prepare()
        self.checkTbColTypeCondition()
        self.queryTbnameCondition()
        self.checkStbTagTypeCondition()
        
    def stop(self):
        tdSql.close()
        tdLog.success("%s successfully executed" % __file__)

tdCases.addWindows(__file__, TDTestCase())
tdCases.addLinux(__file__, TDTestCase())
