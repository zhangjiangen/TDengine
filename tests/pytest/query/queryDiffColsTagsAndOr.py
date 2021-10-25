###################################################################
#           Copyright (c) 2016 by TAOS Technologies, Inc.
#                     All rights reserved.
#
#  This file is proprietary and confidential to TAOS Technologies.
#  No part of this file may be reproduced, stored, transmitted,
#  disclosed or used in any form or by any means other than as
#  expressly provided by the written permission from Jianhui Tao
#
###################################################################

# -*- coding: utf-8 -*-
from copy import deepcopy
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

    def queryLastC10(self, query_sql, multi=False):
        if multi:
            res = tdSql.query(query_sql.replace('c10', 'last(*)'), True)
        else:
            res = tdSql.query(query_sql.replace('*', 'last(*)'), True)
        return int(res[0][-1])

    def queryTsCol(self, tb_name):
        # ts and ts
        query_sql = f'select * from {tb_name} where ts > "2021-01-11 12:00:00" or ts < "2021-01-13 12:00:00"'
        tdSql.query(query_sql)
        tdSql.checkRows(11)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        query_sql = f'select * from {tb_name} where ts >= "2021-01-11 12:00:00" and ts <= "2021-01-13 12:00:00"'
        tdSql.query(query_sql)
        tdSql.checkRows(2)
        tdSql.checkEqual(self.queryLastC10(query_sql), 6)

        ## ts or and tinyint col
        query_sql = f'select * from {tb_name} where ts > "2021-01-11 12:00:00" or c1 = 2'
        tdSql.error(query_sql)

        query_sql = f'select * from {tb_name} where ts <= "2021-01-11 12:00:00" and c1 != 2'
        tdSql.query(query_sql)
        tdSql.checkRows(4)
        tdSql.checkEqual(self.queryLastC10(query_sql), 5)

        ## ts or and smallint col
        query_sql = f'select * from {tb_name} where ts <> "2021-01-11 12:00:00" or c2 = 10'
        tdSql.error(query_sql)

        query_sql = f'select * from {tb_name} where ts <= "2021-01-11 12:00:00" and c2 <= 1'
        tdSql.query(query_sql)
        tdSql.checkRows(1)
        tdSql.checkEqual(self.queryLastC10(query_sql), 1)

        ## ts or and int col
        query_sql = f'select * from {tb_name} where ts >= "2021-01-11 12:00:00" or c3 = 4'
        tdSql.error(query_sql)

        query_sql = f'select * from {tb_name} where ts < "2021-01-11 12:00:00" and c3 = 4'
        tdSql.query(query_sql)
        tdSql.checkRows(1)
        tdSql.checkEqual(self.queryLastC10(query_sql), 4)

        ## ts or and big col
        query_sql = f'select * from {tb_name} where ts is Null or c4 = 5'
        tdSql.error(query_sql)

        query_sql = f'select * from {tb_name} where ts is not Null and c4 = 2'
        tdSql.query(query_sql)
        tdSql.checkRows(1)
        tdSql.checkEqual(self.queryLastC10(query_sql), 3)

        ## ts or and float col
        query_sql = f'select * from {tb_name} where ts between "2021-01-17 12:00:00" and "2021-01-23 12:00:00" or c5 = 6.6'
        tdSql.error(query_sql)

        query_sql = f'select * from {tb_name} where ts < "2021-01-11 12:00:00" and c5 = 1.1'
        tdSql.query(query_sql)
        tdSql.checkRows(4)
        tdSql.checkEqual(self.queryLastC10(query_sql), 4)

        ## ts or and double col
        query_sql = f'select * from {tb_name} where ts between "2021-01-17 12:00:00" and "2021-01-23 12:00:00" or c6 = 7.7'
        tdSql.error(query_sql) 

        query_sql = f'select * from {tb_name} where ts < "2021-01-11 12:00:00" and c6 = 1.1'
        tdSql.query(query_sql)
        tdSql.checkRows(4)
        tdSql.checkEqual(self.queryLastC10(query_sql), 4)

        ## ts or and binary col
        query_sql = f'select * from {tb_name} where ts < "2021-01-11 12:00:00" or c7 like "binary_"'
        tdSql.error(query_sql)

        query_sql = f'select * from {tb_name} where ts <= "2021-01-11 12:00:00" and c7 in ("binary")'
        tdSql.query(query_sql)
        tdSql.checkRows(5)
        tdSql.checkEqual(self.queryLastC10(query_sql), 5)

        ## ts or and nchar col
        query_sql = f'select * from {tb_name} where ts < "2021-01-11 12:00:00" or c8 like "nchar%"'
        tdSql.error(query_sql)

        query_sql = f'select * from {tb_name} where ts >= "2021-01-11 12:00:00" and c8 is Null'
        tdSql.query(query_sql)
        tdSql.checkRows(1)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        ## ts or and bool col
        query_sql = f'select * from {tb_name} where ts < "2021-01-11 12:00:00" or c9=false'
        tdSql.error(query_sql)

        query_sql = f'select * from {tb_name} where ts >= "2021-01-11 12:00:00" and c9=true'
        tdSql.query(query_sql)
        tdSql.checkRows(5)
        tdSql.checkEqual(self.queryLastC10(query_sql), 9)

        ## multi cols
        query_sql = f'select * from {tb_name} where ts > "2021-01-03 12:00:00" and c1 != 2 and c2 >= 2 and c3 <> 4 and c4 < 4 and c5 > 1 and c6 >= 1.1 and c7 is not Null and c8 = "nchar" and c9=false'
        tdSql.query(query_sql)
        tdSql.checkRows(1)
        tdSql.checkEqual(self.queryLastC10(query_sql), 10)

    def queryTsTag(self, tb_name):
        ## ts and tinyint col
        query_sql = f'select * from {tb_name} where ts <= "2021-01-11 12:00:00" and t1 != 2'
        tdSql.query(query_sql)
        tdSql.checkRows(20)

        ## ts and smallint col
        query_sql = f'select * from {tb_name} where ts <= "2021-01-11 12:00:00" and t2 <= 1'
        tdSql.query(query_sql)
        tdSql.checkRows(5)

        ## ts or and int col
        query_sql = f'select * from {tb_name} where ts < "2021-01-11 12:00:00" and t3 = 4'
        tdSql.query(query_sql)
        tdSql.checkRows(4)

        ## ts or and big col
        query_sql = f'select * from {tb_name} where ts is not Null and t4 = 2'
        tdSql.query(query_sql)
        tdSql.checkRows(11)

        ## ts or and float col
        query_sql = f'select * from {tb_name} where ts < "2021-01-11 12:00:00" and t5 = 1.1'
        tdSql.query(query_sql)
        tdSql.checkRows(4)

        ## ts or and double col
        query_sql = f'select * from {tb_name} where ts < "2021-01-11 12:00:00" and t6 = 1.1'
        tdSql.query(query_sql)
        tdSql.checkRows(4)
        tdSql.checkEqual(self.queryLastC10(query_sql), 4)

        ## ts or and binary col
        query_sql = f'select * from {tb_name} where ts <= "2021-01-11 12:00:00" and t7 in ("binary1")'
        tdSql.query(query_sql)
        tdSql.checkRows(5)

        ## ts or and nchar col
        query_sql = f'select * from {tb_name} where ts >= "2021-01-11 12:00:00" and t8 is not Null'
        tdSql.query(query_sql)
        tdSql.checkRows(35)

        ## ts or and bool col
        query_sql = f'select * from {tb_name} where ts >= "2021-01-11 12:00:00" and t9=true'
        tdSql.query(query_sql)
        tdSql.checkRows(35)

        ## multi cols
        query_sql = f'select * from {tb_name} where ts > "2021-01-03 12:00:00" and t1 != 2 and t2 >= 2 and t3 <> 4 and t4 < 4 and t5 > 1 and t6 >= 1.1 and t7 is not Null and t8 = "nchar3" and t9=true'
        tdSql.query(query_sql)
        tdSql.checkRows(10)

    def queryTsColTag(self, tb_name):
        ## ts and tinyint col tag
        query_sql = f'select * from {tb_name} where ts <= "2021-01-11 12:00:00" and c1 >= 2 and t1 != 2'
        tdSql.query(query_sql)
        tdSql.checkRows(4)

        ## ts and smallint col tag
        query_sql = f'select * from {tb_name} where ts <= "2021-01-11 12:00:00" and c2 >=3 and t2 <= 1'
        tdSql.query(query_sql)
        tdSql.checkRows(1)

        ## ts or and int col tag
        query_sql = f'select * from {tb_name} where ts < "2021-01-11 12:00:00" and c3 < 3 and t3 = 4'
        tdSql.query(query_sql)
        tdSql.checkRows(3)

        ## ts or and big col tag
        query_sql = f'select * from {tb_name} where ts is not Null and c4 <> 1 and t4 = 2'
        tdSql.query(query_sql)
        tdSql.checkRows(11)

        ## ts or and float col tag
        query_sql = f'select * from {tb_name} where ts < "2021-01-11 12:00:00" and c5 is not Null and t5 = 1.1'
        tdSql.query(query_sql)
        tdSql.checkRows(4)

        ## ts or and double col tag
        query_sql = f'select * from {tb_name} where ts < "2021-01-11 12:00:00"and c6 = 1.1 and t6 = 1.1'
        tdSql.query(query_sql)
        tdSql.checkRows(4)
        tdSql.checkEqual(self.queryLastC10(query_sql), 4)

        ## ts or and binary col tag
        query_sql = f'select * from {tb_name} where ts <= "2021-01-11 12:00:00" and c7 is Null and t7 in ("binary1")'
        tdSql.query(query_sql)
        tdSql.checkRows(0)

        ## ts or and nchar col tag
        query_sql = f'select * from {tb_name} where ts >= "2021-01-11 12:00:00" and c8 like "nch%" and t8 is not Null'
        tdSql.query(query_sql)
        tdSql.checkRows(30)

        ## ts or and bool col tag
        query_sql = f'select * from {tb_name} where ts >= "2021-01-11 12:00:00" and c9=false and t9=true'
        tdSql.query(query_sql)
        tdSql.checkRows(10)

        ## multi cols tag
        query_sql = f'select * from {tb_name} where ts > "2021-01-03 12:00:00" and c1 = 1 and c2 != 3 and c3 <= 2 and c4 >= 2 and c5 in (1.2, 1.1)  and c6 < 2.2 and c7 like "bina%" and c8 is not Null and c9 = true and t1 != 2 and t2 >= 2 and t3 <> 4 and t4 < 4 and t5 > 1 and t6 >= 1.1 and t7 is not Null and t8 = "nchar3" and t9=true'
        tdSql.query(query_sql)
        tdSql.checkRows(2)

    def queryFullColType(self, tb_name):
        ## != or and
        query_sql = f'select * from {tb_name} where c1 != 1 or c2 = 3'
        tdSql.query(query_sql)
        tdSql.checkRows(3)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        query_sql = f'select * from {tb_name} where c1 != 1 and c2 = 2'
        tdSql.query(query_sql)
        tdSql.checkRows(1)
        tdSql.checkEqual(self.queryLastC10(query_sql), 2)

        ## <> or and
        query_sql = f'select * from {tb_name} where c1 <> 1 or c3 = 3'
        tdSql.query(query_sql)
        tdSql.checkRows(1)
        tdSql.checkEqual(self.queryLastC10(query_sql), 2)

        query_sql = f'select * from {tb_name} where c1 <> 2 and c3 = 4'
        tdSql.query(query_sql)
        tdSql.checkRows(1)
        tdSql.checkEqual(self.queryLastC10(query_sql), 4)

        ## >= or and
        query_sql = f'select * from {tb_name} where c1 >= 2 or c3 = 4'
        tdSql.query(query_sql)
        tdSql.checkRows(2)
        tdSql.checkEqual(self.queryLastC10(query_sql), 4)

        query_sql = f'select * from {tb_name} where c1 >= 2 and c3 = 1'
        tdSql.query(query_sql)
        tdSql.checkRows(1)
        tdSql.checkEqual(self.queryLastC10(query_sql), 2)

        ## <= or and
        query_sql = f'select * from {tb_name} where c1 <= 1 or c3 = 4'
        tdSql.query(query_sql)
        tdSql.checkRows(10)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        query_sql = f'select * from {tb_name} where c1 <= 1 and c3 = 4'
        tdSql.query(query_sql)
        tdSql.checkRows(1)
        tdSql.checkEqual(self.queryLastC10(query_sql), 4)

        ## <> or and is Null
        query_sql = f'select * from {tb_name} where c1 <> 1 or c7 is Null'
        tdSql.query(query_sql)
        tdSql.checkRows(2)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        query_sql = f'select * from {tb_name} where c1 <> 2 and c7 is Null'
        tdSql.query(query_sql)
        tdSql.checkRows(1)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        ## > or and is not Null
        query_sql = f'select * from {tb_name} where c2 > 2 or c8 is not Null'
        tdSql.query(query_sql)
        tdSql.checkRows(11)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        query_sql = f'select * from {tb_name} where c2 > 2 and c8 is not Null'
        tdSql.query(query_sql)
        tdSql.checkRows(1)
        tdSql.checkEqual(self.queryLastC10(query_sql), 3)

        ## > or < or >= or <= or != or <> or = Null
        query_sql = f'select * from {tb_name} where c1 > 1 or c2 < 2 or c3 >= 4 or c4 <= 2 or c5 != 1.1 or c6 <> 1.1 or c7 is Null'
        tdSql.query(query_sql)
        tdSql.checkRows(8)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        query_sql = f'select * from {tb_name} where c1 = 1 and c2 > 1 and c3 >= 1 and c4 <= 5 and c5 != 6.6 and c6 <> 7.7 and c7 is Null'
        tdSql.query(query_sql)
        tdSql.checkRows(1)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        ## tiny small int big or
        query_sql = f'select * from {tb_name} where c1 = 2 or c2 = 3 or c3 = 4 or c4 = 5'
        tdSql.query(query_sql)
        tdSql.checkRows(5)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        query_sql = f'select * from {tb_name} where c1 = 1 and c2 = 2 and c3 = 1 and c4 = 3'
        tdSql.query(query_sql)
        tdSql.checkRows(5)
        tdSql.checkEqual(self.queryLastC10(query_sql), 10)

        ## float double binary nchar bool or
        query_sql = f'select * from {tb_name} where c5=6.6 or c6=7.7 or c7="binary8" or c8="nchar9" or c9=false'
        tdSql.query(query_sql)
        tdSql.checkRows(6)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        query_sql = f'select * from {tb_name} where c5=1.1 and c6=7.7 and c7="binary" and c8="nchar" and c9=true'
        tdSql.query(query_sql)
        tdSql.checkRows(1)
        tdSql.checkEqual(self.queryLastC10(query_sql), 7)

        ## all types or
        query_sql = f'select * from {tb_name} where c1=2 or c2=3 or c3=4 or c4=5 or c5=6.6 or c6=7.7 or c7 nmatch "binary[134]" or c8="nchar9" or c9=false'
        tdSql.query(query_sql)
        tdSql.checkRows(11)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        query_sql = f'select * from {tb_name} where c1=1 and c2=2 and c3=1 and c4=3 and c5=1.1 and c6=1.1 and c7 match "binary[28]" and c8 in ("nchar") and c9=true'
        tdSql.query(query_sql)
        tdSql.checkRows(1)
        tdSql.checkEqual(self.queryLastC10(query_sql), 8)

        query_sql = f'select * from {tb_name} where c1=1 and c2=2 or c3=1 and c4=3 and c5=1.1 and c6=1.1 and c7 match "binary[28]" and c8 in ("nchar") and c9=true'
        tdSql.query(query_sql)
        tdSql.checkRows(7)
        tdSql.checkEqual(self.queryLastC10(query_sql), 10)

    def queryFullTagType(self, tb_name):
        ## != or and
        query_sql = f'select * from {tb_name} where t1 != 1 or t2 = 3'
        tdSql.query(query_sql)
        tdSql.checkRows(44)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        query_sql = f'select * from {tb_name} where t1 != 1 and t2 = 2'
        tdSql.query(query_sql)
        tdSql.checkRows(11)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        ## <> or and
        query_sql = f'select * from {tb_name} where t1 <> 1 or t3 = 3'
        tdSql.query(query_sql)
        tdSql.checkRows(44)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        query_sql = f'select * from {tb_name} where t1 <> 2 and t3 = 4'
        tdSql.query(query_sql)
        tdSql.checkRows(11)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        ## >= or and
        query_sql = f'select * from {tb_name} where t1 >= 2 or t3 = 4'
        tdSql.query(query_sql)
        tdSql.checkRows(44)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        query_sql = f'select * from {tb_name} where t1 >= 1 and t3 = 1'
        tdSql.query(query_sql)
        tdSql.checkRows(11)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        ## <= or and
        query_sql = f'select * from {tb_name} where t1 <= 1 or t3 = 4'
        tdSql.query(query_sql)
        tdSql.checkRows(22)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        query_sql = f'select * from {tb_name} where t1 <= 3 and t3 = 2'
        tdSql.query(query_sql)
        tdSql.checkRows(11)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        ## <> or and is Null
        query_sql = f'select * from {tb_name} where t1 <> 1 or t7 is Null'
        tdSql.query(query_sql)
        tdSql.checkRows(44)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        query_sql = f'select * from {tb_name} where t1 <> 2 and t7 is not Null'
        tdSql.query(query_sql)
        tdSql.checkRows(44)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        ## > or and is not Null
        query_sql = f'select * from {tb_name} where t2 > 2 or t8 is not Null'
        tdSql.query(query_sql)
        tdSql.checkRows(55)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        query_sql = f'select * from {tb_name} where t2 > 2 and t8 is not Null'
        tdSql.query(query_sql)
        tdSql.checkRows(33)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        ## > or < or >= or <= or != or <> or = Null
        query_sql = f'select * from {tb_name} where t1 > 1 or t2 < 2 or t3 >= 4 or t4 <= 2 or t5 != 1.1 or t6 <> 1.1 or t7 is Null'
        tdSql.query(query_sql)
        tdSql.checkRows(55)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        query_sql = f'select * from {tb_name} where t1 >= 1 and t2 > 1 and t3 >= 1 and t4 <= 5 and t5 != 6.6 and t6 <> 7.7 and t7 is not Null'
        tdSql.query(query_sql)
        tdSql.checkRows(44)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        ## tiny small int big or and
        query_sql = f'select * from {tb_name} where t1 = 2 or t2 = 3 or t3 = 4 or t4 = 5'
        tdSql.query(query_sql)
        tdSql.checkRows(44)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        query_sql = f'select * from {tb_name} where t1 = 1 and t2 = 2 and t3 = 1 and t4 = 3'
        tdSql.query(query_sql)
        tdSql.checkRows(0)

        ## float double binary nchar bool or and
        query_sql = f'select * from {tb_name} where t5=2.2 or t6=7.7 or t7="binary8" or t8="nchar9" or t9=false'
        tdSql.query(query_sql)
        tdSql.checkRows(11)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        query_sql = f'select * from {tb_name} where t5=2.2 and t6=2.2 and t7="binary2" and t8="nchar2" and t9=true'
        tdSql.query(query_sql)
        tdSql.checkRows(11)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        ## all types or and
        query_sql = f'select * from {tb_name} where t1=2 or t2=3 or t3=4 or t4=5 or t5=6.6 or t6=7.7 or t7 nmatch "binary[134]" or t8="nchar9" or t9=false'
        tdSql.query(query_sql)
        tdSql.checkRows(44)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        query_sql = f'select * from {tb_name} where t1=1 and t2=1 and t3>=1 and t4!=2 and t5=1.1 and t6=1.1 and t7 match "binary[18]" and t8 in ("nchar1") and t9=true'
        tdSql.query(query_sql)
        tdSql.checkRows(11)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

        query_sql = f'select * from {tb_name} where t1=1 and t2=1 or t3>=1 and t4!=2 and t5=1.1 and t6=1.1 and t7 match "binary[18]" and t8 in ("nchar1") and t9=true'
        tdSql.query(query_sql)
        tdSql.checkRows(11)
        tdSql.checkEqual(self.queryLastC10(query_sql), 11)

    def checkTbColTypeOperator(self):
        '''
            Ordinary table full column type and operator
        '''
        tb_name = self.initTb()
        self.queryFullColType(tb_name)

    def checkStbColTypeOperator(self):
        '''
            Super table full column type and operator
        '''
        tb_name = self.initStb()
        self.queryFullColType(f'{tb_name}_sub_1')

    
    def checkStbTagTypeOperator(self):
        '''
            Super table full tag type and operator
        '''
        tb_name = self.initStb()
        self.queryFullTagType(tb_name)

    def checkTbTsCol(self):
        '''
            Ordinary table ts and col check
        '''
        tb_name = self.initTb()
        self.queryTsCol(tb_name)

    def checkStbTsTol(self):
        tb_name = self.initStb()
        self.queryTsCol(f'{tb_name}_sub_1')

    def checkStbTsTag(self):
        tb_name = self.initStb()
        self.queryTsTag(tb_name)

    def checkStbTsColTag(self):
        tb_name = self.initStb()
        self.queryTsColTag(tb_name)
   

    def run(self):
        tdSql.prepare()
        # self.checkTbColTypeOperator()
        # self.checkStbColTypeOperator()
        # self.checkStbTagTypeOperator()
        # self.checkTbTsCol()
        # self.checkStbTsTol()
        # self.checkStbTsTag()
        # self.checkStbTsColTag()
       

    def stop(self):
        tdSql.close()
        tdLog.success("%s successfully executed" % __file__)


tdCases.addWindows(__file__, TDTestCase())
tdCases.addLinux(__file__, TDTestCase())
