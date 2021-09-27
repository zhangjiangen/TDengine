###################################################################
#           Copyright (c) 2021 by TAOS Technologies, Inc.
#                     All rights reserved.
#
#  This file is proprietary and confidential to TAOS Technologies.
#  No part of this file may be reproduced, stored, transmitted,
#  disclosed or used in any form or by any means other than as
#  expressly provided by the written permission from Jianhui Tao
#
###################################################################

# -*- coding: utf-8 -*-

import traceback
import random
from taos.error import LinesError
import time
from copy import deepcopy
import numpy as np
from util.log import *
from util.cases import *
from util.sql import *
from util.common import tdCom
import threading
import json

class TDTestCase:
    def init(self, conn, logSql):
        tdLog.debug("start to execute %s" % __file__)
        tdSql.init(conn.cursor(), logSql)
        self._conn = conn 

    def createDb(self, name="test", db_update_tag=0):
        if db_update_tag == 0:
            tdSql.execute(f"drop database if exists {name}")
            tdSql.execute(f"create database if not exists {name} precision 'us'")
        else:
            tdSql.execute(f"drop database if exists {name}")
            tdSql.execute(f"create database if not exists {name} precision 'us' update 1")
        tdSql.execute(f'use {name}')

    def timeTrans(self, ts_value):
        if type(ts_value) is int:
            if ts_value != 0:
                ts = ts_value/1000000
            else:
                ts = time.time()
        elif type(ts_value) is dict:
            if ts_value["type"].lower() == "ns":
                ts = ts_value["value"]/1000000000
            elif ts_value["type"].lower() == "us":
                ts = ts_value["value"]/1000000
            elif ts_value["type"].lower() == "ms":
                ts = ts_value["value"]/1000
            elif ts_value["type"].lower() == "s":
                ts = ts_value["value"]/1
            else:
                ts = ts_value["value"]/1000000
        else:
            print("input ts maybe not right format")
        ulsec = repr(ts).split('.')[1][:6]
        if len(ulsec) < 6 and int(ulsec) != 0:
            ulsec = int(ulsec) * (10 ** (6 - len(ulsec)))
        elif int(ulsec) == 0:
            ulsec *= 6
            # * follow two rows added for tsCheckCase
            td_ts = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(ts))
            return td_ts
        #td_ts = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(ts))
        td_ts = time.strftime("%Y-%m-%d %H:%M:%S.{}".format(ulsec), time.localtime(ts))
        return td_ts

    def dateToTs(self, datetime_input):
        return int(time.mktime(time.strptime(datetime_input, "%Y-%m-%d %H:%M:%S.%f")))

    def typeTrans(self, type_list):
        type_num_list = []
        for tp in type_list:
            if type(tp) is dict:
                tp = tp['type']
            if tp.upper() == "TIMESTAMP":
                type_num_list.append(9)
            elif tp.upper() == "BOOL":
                type_num_list.append(1)
            elif tp.upper() == "TINYINT":
                type_num_list.append(2)
            elif tp.upper() == "SMALLINT":
                type_num_list.append(3)
            elif tp.upper() == "INT":
                type_num_list.append(4)
            elif tp.upper() == "BIGINT":
                type_num_list.append(5)
            elif tp.upper() == "FLOAT":
                type_num_list.append(6)
            elif tp.upper() == "DOUBLE":
                type_num_list.append(7)
            elif tp.upper() == "BINARY":
                type_num_list.append(8)
            elif tp.upper() == "NCHAR":
                type_num_list.append(10)
            elif tp.upper() == "BIGINT UNSIGNED":
                type_num_list.append(14)
        return type_num_list

    def inputHandle(self, input_json):
        stb_name = input_json["metric"]
        stb_tag_dict = input_json["tags"]
        stb_col_dict = input_json["value"]
        ts_value = self.timeTrans(input_json["timestamp"])

        tag_name_list = []
        tag_value_list = []
        td_tag_value_list = []
        td_tag_type_list = []

        col_name_list = []
        col_value_list = []
        td_col_value_list = []
        td_col_type_list = []

        # handle tag
        for key,value in stb_tag_dict.items():
            if "id" in key.lower():
                tb_name = value
            else:
                if type(value) is dict:
                    tag_value_list.append(str(value["value"]))
                    td_tag_value_list.append(str(value["value"]))
                    tag_name_list.append(key)
                    td_tag_type_list.append(value["type"].upper())
                    tb_name = ""
                else:
                    tag_value_list.append(str(value))
                    td_tag_value_list.append(str(value))
                    tag_name_list.append(key)
                    tb_name = ""

                    if type(value) is bool:
                        td_tag_type_list.append("BOOL")
                    elif type(value) is int:
                        td_tag_type_list.append("BIGINT")
                    elif type(value) is float:
                        td_tag_type_list.append("DOUBLE")
                    elif type(value) is str:
                        if tdSql.getVariable("defaultJSONStrType")[0].lower() == "nchar":
                            td_tag_type_list.append("NCHAR")
                        else:
                            td_tag_type_list.append("BINARY")

        # handle col
        if type(stb_col_dict) is dict:
            if stb_col_dict["type"].lower() == "bool":
                bool_value = f'{stb_col_dict["value"]}'
                col_value_list.append(bool_value)
                td_col_type_list.append(stb_col_dict["type"].upper())
                col_name_list.append("value")
                td_col_value_list.append(str(stb_col_dict["value"]))
            else:
                col_value_list.append(stb_col_dict["value"])
                td_col_type_list.append(stb_col_dict["type"].upper())
                col_name_list.append("value")
                td_col_value_list.append(str(stb_col_dict["value"]))
        else:
            col_name_list.append("value")
            col_value_list.append(str(stb_col_dict))
            td_col_value_list.append(str(stb_col_dict))
            # td_col_type_list.append(tdCom.typeof(stb_col_dict).upper())
            if type(stb_col_dict) is bool:
                td_col_type_list.append("BOOL")
            elif type(stb_col_dict) is int:
                td_col_type_list.append("BIGINT")
            elif type(stb_col_dict) is float:
                td_col_type_list.append("DOUBLE")
            elif type(stb_col_dict) is str:
                if tdSql.getVariable("defaultJSONStrType")[0].lower() == "nchar":
                    td_col_type_list.append("NCHAR")
                else:
                    td_col_type_list.append("BINARY")
        # else:
        #     # handle tag
        #     for key,value in stb_tag_dict.items():
        #         if "id" in key.lower():
        #             tb_name = value
        #         else:
        #             if type(value) is dict:
        #                 tag_value_list.append(str(value["value"]))
        #                 td_tag_value_list.append(str(value["value"]))
        #                 tag_name_list.append(key)
        #                 td_tag_type_list.append(value["type"].upper())
        #                 tb_name = ""
        #             else:
        #                 if type(value) is bool:
        #                     td_tag_type_list.append("BOOL")
        #                 elif type(value) is int:
        #                     td_tag_type_list.append("BIGINT")
        #                 elif type(value) is float:
        #                     td_tag_type_list.append("DOUBLE")
        #                 elif type(value) is str:
        #                     td_tag_type_list.append("NCHAR")
        #                 tag_value_list.append(str(value))
        #                 td_tag_value_list.append(str(value))
        #                 tag_name_list.append(key)
        #                 tb_name = ""

            
        final_field_list = []
        final_field_list.extend(col_name_list)
        final_field_list.extend(tag_name_list)

        final_type_list = []
        final_type_list.append("TIMESTAMP")
        final_type_list.extend(td_col_type_list)
        final_type_list.extend(td_tag_type_list)
        final_type_list = self.typeTrans(final_type_list)

        final_value_list = []
        final_value_list.append(ts_value)
        final_value_list.extend(td_col_value_list)
        final_value_list.extend(td_tag_value_list)
        return final_value_list, final_field_list, final_type_list, stb_name, tb_name

    def genTsColValue(self, value, t_type=None, value_type="obj"):
        if value_type == "obj":
            if t_type == None:
                ts_col_value = value
            else:
                ts_col_value = {"value": value, "type":	t_type}
        elif value_type == "default":
            ts_col_value = value
        return ts_col_value

    def genTagValue(self, t0_type="bool", t0_value="", t1_type="tinyint", t1_value=127, t2_type="smallint", t2_value=32767,
                    t3_type="int", t3_value=2147483647, t4_type="bigint", t4_value=9223372036854775807,
                    t5_type="float", t5_value=11.12345027923584, t6_type="double", t6_value=22.123456789, 
                    t7_type="binary", t7_value="binaryTagValue", t8_type="nchar", t8_value="ncharTagValue", value_type="obj"):
        if t0_value == "":
            t0_value = random.choice([True, False])
        if value_type == "obj":
            tag_value = {
                "t0": {"value": t0_value, "type": t0_type},
                "t1": {"value": t1_value, "type": t1_type},
                "t2": {"value": t2_value, "type": t2_type},
                "t3": {"value": t3_value, "type": t3_type},
                "t4": {"value": t4_value, "type": t4_type},
                "t5": {"value": t5_value, "type": t5_type},
                "t6": {"value": t6_value, "type": t6_type},
                "t7": {"value": t7_value, "type": t7_type},
                "t8": {"value": t8_value, "type": t8_type}
            }
        elif value_type == "default":
            tag_value = {
                "t0": t0_value,
                "t1": t1_value,
                "t2": t2_value,
                "t3": t3_value,
                "t4": t4_value,
                "t6": t6_value,
                "t7": t7_value,
                "t8": t8_value
            }
        return tag_value

    def genFullTypeJson(self, ts_value="", col_value="", tag_value="", stb_name="", tb_name="", 
                        id_noexist_tag=None, id_change_tag=None, id_upper_tag=None, id_double_tag=None,
                        t_add_tag=None, t_mul_tag=None, c_multi_tag=None, c_blank_tag=None, t_blank_tag=None, 
                        chinese_tag=None, multi_field_tag=None, point_trans_tag=None, value_type="obj"):
        if value_type == "obj":
            if stb_name == "":
                stb_name = tdCom.getLongName(len=6, mode="letters")
            if tb_name == "":
                tb_name = f'{stb_name}_{random.randint(0, 65535)}_{random.randint(0, 65535)}'
            if ts_value == "":
                ts_value = self.genTsColValue(1626006833639000000, "ns")
            if col_value == "":
                col_value = self.genTsColValue(random.choice([True, False]), "bool")
            if tag_value == "":
                tag_value = self.genTagValue()
            if id_upper_tag is not None:
                id = "ID"
            else:
                id = "id"
            if id_noexist_tag is None:
                tag_value[id] = tb_name
            sql_json = {"metric": stb_name, "timestamp": ts_value, "value": col_value, "tags": tag_value}
            if id_noexist_tag is not None:
                if t_add_tag is not None:
                    tag_value["t9"] = {"value": "ncharTagValue", "type": "nchar"}
                    sql_json = {"metric": stb_name, "timestamp": ts_value, "value": col_value, "tags": tag_value}
            if id_change_tag is not None:
                tag_value.pop('t8')
                tag_value["t8"] = {"value": "ncharTagValue", "type": "nchar"}
                sql_json = {"metric": stb_name, "timestamp": ts_value, "value": col_value, "tags": tag_value}
            if id_double_tag is not None:
                tag_value["ID"] = f'"{tb_name}_2"'
                sql_json = {"metric": stb_name, "timestamp": ts_value, "value": col_value, "tags": tag_value}
            if t_add_tag is not None:
                tag_value["t10"] = {"value": "ncharTagValue", "type": "nchar"}
                tag_value["t11"] = {"value": True, "type": "bool"}
                sql_json = {"metric": stb_name, "timestamp": ts_value, "value": col_value, "tags": tag_value}
            if t_mul_tag is not None:
                tag_value.pop('t8')
                sql_json = {"metric": stb_name, "timestamp": ts_value, "value": col_value, "tags": tag_value}
            if c_multi_tag is not None:
                col_value = [{"value": True, "type": "bool"}, {"value": False, "type": "bool"}]
                sql_json = {"metric": stb_name, "timestamp": ts_value, "value": col_value, "tags": tag_value}
            if t_blank_tag is not None:
                tag_value = {"id": tdCom.getLongName(len=6, mode="letters")}
                sql_json = {"metric": stb_name, "timestamp": ts_value, "value": col_value, "tags": tag_value}
            if chinese_tag is not None:
                tag_value = {"id": "abc", "t0": {"value": "涛思数据", "type": "nchar"}}
                col_value = {"value": "涛思数据", "type": "nchar"}
                sql_json = {"metric": stb_name, "timestamp": ts_value, "value": col_value, "tags": tag_value}
            if c_blank_tag is not None:
                sql_json.pop("value")
            if multi_field_tag is not None:
                sql_json = {"metric": stb_name, "timestamp": ts_value, "value": col_value, "tags": tag_value, "tags2": tag_value}
            if point_trans_tag is not None:
                sql_json = {"metric": ".point.trans.test", "timestamp": ts_value, "value": col_value, "tags": tag_value}

        elif value_type == "default":
            if stb_name == "":
                stb_name = tdCom.getLongName(len=6, mode="letters")
            if tb_name == "":
                tb_name = f'{stb_name}_{random.randint(0, 65535)}_{random.randint(0, 65535)}'
            if ts_value == "":
                ts_value = 1626006833639018
            if col_value == "":
                col_value = random.choice([True, False])
            if tag_value == "":
                tag_value = self.genTagValue(value_type=value_type)
            if id_upper_tag is not None:
                id = "ID"
            else:
                id = "id"
            if id_noexist_tag is None:
                tag_value[id] = tb_name
            sql_json = {"metric": stb_name, "timestamp": ts_value, "value": col_value, "tags": tag_value}
            if id_noexist_tag is not None:
                if t_add_tag is not None:
                    tag_value["t9"] = {"value": "ncharTagValue", "type": "nchar"}
                    sql_json = {"metric": stb_name, "timestamp": ts_value, "value": col_value, "tags": tag_value}
            if id_change_tag is not None:
                tag_value.pop('t7')
                tag_value["t7"] = {"value": "ncharTagValue", "type": "nchar"}
                sql_json = {"metric": stb_name, "timestamp": ts_value, "value": col_value, "tags": tag_value}
            if id_double_tag is not None:
                tag_value["ID"] = f'"{tb_name}_2"'
                sql_json = {"metric": stb_name, "timestamp": ts_value, "value": col_value, "tags": tag_value}
            if t_add_tag is not None:
                tag_value["t10"] = {"value": "ncharTagValue", "type": "nchar"}
                tag_value["t11"] = True
                sql_json = {"metric": stb_name, "timestamp": ts_value, "value": col_value, "tags": tag_value}
            if t_mul_tag is not None:
                tag_value.pop('t7')
                sql_json = {"metric": stb_name, "timestamp": ts_value, "value": col_value, "tags": tag_value}
            if c_multi_tag is not None:
                col_value = True,False
                sql_json = {"metric": stb_name, "timestamp": ts_value, "value": col_value, "tags": tag_value}
            if t_blank_tag is not None:
                tag_value = {"id": tdCom.getLongName(len=6, mode="letters")}
                sql_json = {"metric": stb_name, "timestamp": ts_value, "value": col_value, "tags": tag_value}
            if c_blank_tag is not None:
                sql_json.pop("value")
            if multi_field_tag is not None:
                sql_json = {"metric": stb_name, "timestamp": ts_value, "value": col_value, "tags": tag_value, "tags2": tag_value}
            if point_trans_tag is not None:
                sql_json = {"metric": ".point.trans.test", "timestamp": ts_value, "value": col_value, "tags": tag_value}
        return sql_json, stb_name
    
    def genMulTagColDict(self, genType, count=1, value_type="obj"):
        """
            genType must be tag/col
        """
        tag_dict = dict()
        col_dict = dict()
        if value_type == "obj":
            if genType == "tag":
                for i in range(0, count):
                    tag_dict[f't{i}'] = {'value': True, 'type': 'bool'}
                return tag_dict
            if genType == "col":
                col_dict = {'value': True, 'type': 'bool'}
                return col_dict
        elif value_type == "default":
            if genType == "tag":
                for i in range(0, count):
                    tag_dict[f't{i}'] = True
                return tag_dict
            if genType == "col":
                col_dict = True
                return col_dict

    def genLongJson(self, tag_count, value_type="obj"):
        stb_name = tdCom.getLongName(7, mode="letters")
        tb_name = f'{stb_name}_1'
        tag_dict = self.genMulTagColDict("tag", tag_count, value_type)
        col_dict = self.genMulTagColDict("col", 1, value_type)
        tag_dict["id"] = tb_name
        ts_dict = {'value': 1626006833639000000, 'type': 'ns'}
        long_json = {"metric": stb_name, "timestamp": ts_dict, "value": col_dict, "tags": tag_dict}
        return long_json, stb_name

    def getNoIdTbName(self, stb_name):
        query_sql = f"select tbname from {stb_name}"
        tb_name = self.resHandle(query_sql, True)[0][0]
        return tb_name

    def resHandle(self, query_sql, query_tag):
        tdSql.execute('reset query cache')
        row_info = tdSql.query(query_sql, query_tag)
        col_info = tdSql.getColNameList(query_sql, query_tag)
        res_row_list = []
        sub_list = []
        for row_mem in row_info:
            for i in row_mem:
                sub_list.append(str(i))
            res_row_list.append(sub_list)
        res_field_list_without_ts = col_info[0][1:]
        res_type_list = col_info[1]
        return res_row_list, res_field_list_without_ts, res_type_list

    def resCmp(self, input_json, stb_name, query_sql="select * from", condition="", ts=None, id=True, none_check_tag=None):
        expect_list = self.inputHandle(input_json)
        self._conn.insert_lines([json.dumps(input_json)], 2)
        query_sql = f"{query_sql} {stb_name} {condition}"
        res_row_list, res_field_list_without_ts, res_type_list = self.resHandle(query_sql, True)
        if ts == 0:
            res_ts = self.dateToTs(res_row_list[0][0])
            current_time = time.time()
            if current_time - res_ts < 60:
                tdSql.checkEqual(res_row_list[0][1:], expect_list[0][1:])
            else:
                print("timeout")
                tdSql.checkEqual(res_row_list[0], expect_list[0])
        else:
            if none_check_tag is not None:
                none_index_list = [i for i,x in enumerate(res_row_list[0]) if x=="None"]
                none_index_list.reverse()
                for j in none_index_list:
                    res_row_list[0].pop(j)
                    expect_list[0].pop(j)
            tdSql.checkEqual(res_row_list[0], expect_list[0])
        tdSql.checkEqual(res_field_list_without_ts, expect_list[1])
        for i in range(len(res_type_list)):
            tdSql.checkEqual(res_type_list[i], expect_list[2][i])
        # tdSql.checkEqual(res_type_list, expect_list[2])

    def initCheckCase(self, value_type="obj"):
        """
            normal tags and cols, one for every elm
        """
        tdCom.cleanTb()
        input_json, stb_name = self.genFullTypeJson(value_type=value_type)
        self.resCmp(input_json, stb_name)

    def boolTypeCheckCase(self):
        """
            check all normal type
        """
        tdCom.cleanTb()
        full_type_list = ["f", "F", "false", "False", "t", "T", "true", "True"]
        for t_type in full_type_list:
            input_json_list = [self.genFullTypeJson(tag_value=self.genTagValue(t0_value=t_type))[0],
                                self.genFullTypeJson(col_value=self.genTsColValue(value=t_type, t_type="bool"))[0]]
            for input_json in input_json_list:
                try:
                    self._conn.insert_lines([json.dumps(input_json)], 2)
                    raise Exception("should not reach here")
                except LinesError as err:
                    tdSql.checkNotEqual(err.errno, 0)
        
    def symbolsCheckCase(self, value_type="obj"):
        """
            check symbols = `~!@#$%^&*()_-+={[}]\|:;'\",<.>/? 
        """
        '''
            please test :
            binary_symbols = '\"abcd`~!@#$%^&*()_-{[}]|:;<.>?lfjal"\'\'"\"'
        '''
        tdCom.cleanTb()
        binary_symbols = '"abcd`~!@#$%^&*()_-{[}]|:;<.>?lfjal"'
        nchar_symbols = binary_symbols
        input_sql1, stb_name1 = self.genFullTypeJson(col_value=self.genTsColValue(value=binary_symbols, t_type="binary", value_type=value_type), 
                                    tag_value=self.genTagValue(t7_value=binary_symbols, t8_value=nchar_symbols, value_type=value_type))
        input_sql2, stb_name2 = self.genFullTypeJson(col_value=self.genTsColValue(value=nchar_symbols, t_type="nchar", value_type=value_type), 
                                    tag_value=self.genTagValue(t7_value=binary_symbols, t8_value=nchar_symbols, value_type=value_type))
        self.resCmp(input_sql1, stb_name1)
        self.resCmp(input_sql2, stb_name2)

    def tsCheckCase(self, value_type="obj"):
        """
            test ts list --> ["1626006833639000000ns", "1626006833639019us", "1626006833640ms", "1626006834s", "1626006822639022"]
            # ! us级时间戳都为0时，数据库中查询显示，但python接口拿到的结果不显示 .000000的情况请确认，目前修改时间处理代码可以通过
        """
        tdCom.cleanTb()
        ts_list = ["1626006833639000000ns", "1626006833639019us", "1626006833640ms", "1626006834s", "1626006822639022", 0]
        for ts in ts_list:
            if "s" in str(ts):
                input_json, stb_name = self.genFullTypeJson(ts_value=self.genTsColValue(value=int(tdCom.splitNumLetter(ts)[0]), t_type=tdCom.splitNumLetter(ts)[1]))
                self.resCmp(input_json, stb_name, ts=ts)
            else:
                input_json, stb_name = self.genFullTypeJson(ts_value=self.genTsColValue(value=int(ts), t_type="us", value_type=value_type))
                self.resCmp(input_json, stb_name, ts=ts)
                if int(ts) == 0:
                    if value_type == "obj":
                        input_json_list = [self.genFullTypeJson(ts_value=self.genTsColValue(value=int(ts), t_type="")),
                                            self.genFullTypeJson(ts_value=self.genTsColValue(value=int(ts), t_type="ns")),
                                            self.genFullTypeJson(ts_value=self.genTsColValue(value=int(ts), t_type="us")),
                                            self.genFullTypeJson(ts_value=self.genTsColValue(value=int(ts), t_type="ms")),
                                            self.genFullTypeJson(ts_value=self.genTsColValue(value=int(ts), t_type="s"))]
                    elif value_type == "default":
                        input_json_list = [self.genFullTypeJson(ts_value=self.genTsColValue(value=int(ts), value_type=value_type))]
                    for input_json in input_json_list:
                        self.resCmp(input_json[0], input_json[1], ts=ts)
                else:
                    input_json = self.genFullTypeJson(ts_value=self.genTsColValue(value=int(ts), t_type=""))[0]
                    try:
                        self._conn.insert_lines([json.dumps(input_json)], 2)
                        raise Exception("should not reach here")
                    except LinesError as err:
                        tdSql.checkNotEqual(err.errno, 0)
    
    def idSeqCheckCase(self, value_type="obj"):
        """
            check id.index in tags
            eg: t0=**,id=**,t1=**
        """
        tdCom.cleanTb()
        input_json, stb_name = self.genFullTypeJson(id_change_tag=True, value_type=value_type)
        self.resCmp(input_json, stb_name)
    
    def idUpperCheckCase(self, value_type="obj"):
        """
            check id param
            eg: id and ID
        """
        tdCom.cleanTb()
        input_json, stb_name = self.genFullTypeJson(id_upper_tag=True, value_type=value_type)
        self.resCmp(input_json, stb_name)
        input_json, stb_name = self.genFullTypeJson(id_change_tag=True, id_upper_tag=True, value_type=value_type)
        self.resCmp(input_json, stb_name)

    def noIdCheckCase(self, value_type="obj"):
        """
            id not exist
        """
        tdCom.cleanTb()
        input_json, stb_name = self.genFullTypeJson(id_noexist_tag=True, value_type=value_type)
        self.resCmp(input_json, stb_name)
        query_sql = f"select tbname from {stb_name}"
        res_row_list = self.resHandle(query_sql, True)[0]
        if len(res_row_list[0][0]) > 0:
            tdSql.checkColNameList(res_row_list, res_row_list)
        else:
            tdSql.checkColNameList(res_row_list, "please check noIdCheckCase")

    def maxColTagCheckCase(self, value_type="obj"):
        """
            max tag count is 128
        """
        for input_json in [self.genLongJson(128, value_type)[0]]:
            tdCom.cleanTb()
            self._conn.insert_lines([json.dumps(input_json)], 2)
        for input_json in [self.genLongJson(129, value_type)[0]]:
            tdCom.cleanTb()
            try:
                self._conn.insert_lines([json.dumps(input_json)], 2)
            except LinesError as err:
                tdSql.checkNotEqual(err.errno, 0)
            
    def idIllegalNameCheckCase(self, value_type="obj"):
        """
            test illegal id name
            mix "`~!@#$¥%^&*()-+={}|[]、「」【】\:;《》<>?"
        """
        tdCom.cleanTb()
        rstr = list("`~!@#$¥%^&*()-+={}|[]、「」【】\:;《》<>?")
        for i in rstr:
            input_json = self.genFullTypeJson(tb_name=f'aa{i}bb', value_type=value_type)[0]
            try:
                self._conn.insert_lines([json.dumps(input_json)], 2)
            except LinesError as err:
                tdSql.checkNotEqual(err.errno, 0)

    def idStartWithNumCheckCase(self, value_type="obj"):
        """
            id is start with num
        """
        tdCom.cleanTb()
        input_json = self.genFullTypeJson(tb_name="1aaabbb", value_type=value_type)[0]
        try:
            self._conn.insert_lines([json.dumps(input_json)], 2)
        except LinesError as err:
            tdSql.checkNotEqual(err.errno, 0)

    def nowTsCheckCase(self, value_type="obj"):
        """
            check now unsupported
        """
        tdCom.cleanTb()
        input_json = self.genFullTypeJson(ts_value=self.genTsColValue(value="now", t_type="ns", value_type=value_type))[0]
        try:
            self._conn.insert_lines([json.dumps(input_json)], 2)
        except LinesError as err:
            tdSql.checkNotEqual(err.errno, 0)

    def dateFormatTsCheckCase(self, value_type="obj"):
        """
            check date format ts unsupported
        """
        tdCom.cleanTb()
        input_json = self.genFullTypeJson(ts_value=self.genTsColValue(value="2021-07-21\ 19:01:46.920", t_type="ns", value_type=value_type))[0]
        try:
            self._conn.insert_lines([json.dumps(input_json)], 2)
        except LinesError as err:
                tdSql.checkNotEqual(err.errno, 0)
    
    def illegalTsCheckCase(self, value_type="obj"):
        """
            check ts format like 16260068336390us19
        """
        tdCom.cleanTb()
        input_json = self.genFullTypeJson(ts_value=self.genTsColValue(value="16260068336390us19", t_type="us", value_type=value_type))[0]
        try:
            self._conn.insert_lines([json.dumps(input_json)], 2)
        except LinesError as err:
            tdSql.checkNotEqual(err.errno, 0)

    def tagValueLengthCheckCase(self, value_type="obj"):
        """
            check full type tag value limit
        """
        tdCom.cleanTb()
        # i8
        for t1 in [-127, 127]:
            input_json, stb_name = self.genFullTypeJson(tag_value=self.genTagValue(t1_value=t1, value_type=value_type))
            self.resCmp(input_json, stb_name)
        for t1 in [-128, 128]:
            input_json = self.genFullTypeJson(tag_value=self.genTagValue(t1_value=t1))[0]
            print(input_json)
            try:
                self._conn.insert_lines([json.dumps(input_json)], 2)
                raise Exception("should not reach here")
            except LinesError as err:
                tdSql.checkNotEqual(err.errno, 0)

        #i16
        for t2 in [-32767, 32767]:
            input_json, stb_name = self.genFullTypeJson(tag_value=self.genTagValue(t2_value=t2, value_type=value_type))
            self.resCmp(input_json, stb_name)
        for t2 in [-32768, 32768]:
            input_json = self.genFullTypeJson(tag_value=self.genTagValue(t2_value=t2))[0]
            try:
                self._conn.insert_lines([json.dumps(input_json)], 2)
                raise Exception("should not reach here")
            except LinesError as err:
                tdSql.checkNotEqual(err.errno, 0)

        #i32
        for t3 in [-2147483647, 2147483647]:
            input_json, stb_name = self.genFullTypeJson(tag_value=self.genTagValue(t3_value=t3, value_type=value_type))
            self.resCmp(input_json, stb_name)
        for t3 in [-2147483648, 2147483648]:
            input_json = self.genFullTypeJson(tag_value=self.genTagValue(t3_value=t3))[0]
            try:
                self._conn.insert_lines([json.dumps(input_json)], 2)
                raise Exception("should not reach here")
            except LinesError as err:
                tdSql.checkNotEqual(err.errno, 0)

        #i64 
        for t4 in [-9223372036854775807, 9223372036854775807]:
            input_json, stb_name = self.genFullTypeJson(tag_value=self.genTagValue(t4_value=t4, value_type=value_type))
            print(input_json)
            self.resCmp(input_json, stb_name)
            
        for t4 in [-9223372036854775808, 9223372036854775808]:
            input_json = self.genFullTypeJson(tag_value=self.genTagValue(t4_value=t4, value_type=value_type))[0]
            try:
                self._conn.insert_lines([json.dumps(input_json)], 2)
                raise Exception("should not reach here")
            except LinesError as err:
                tdSql.checkNotEqual(err.errno, 0)


        # ! bug
        # # f32
        # for t5 in [-3.4028234663852885981170418348451692544*(10**38), 3.4028234663852885981170418348451692544*(10**38)]:
        #     input_json, stb_name = self.genFullTypeJson(tag_value=self.genTagValue(t5_value=t5, value_type=value_type))
        #     self.resCmp(input_json, stb_name)
        # # * limit set to 3.4028234664*(10**38)
        # for t5 in [-3.4028234664*(10**38), 3.4028234664*(10**38)]:
        #     input_json = self.genFullTypeJson(tag_value=self.genTagValue(t5_value=t5, value_type=value_type))[0]
        #     print(input_json)
        #     try:
        #         self._conn.insert_lines([json.dumps(input_json)], 2)
        #         raise Exception("should not reach here")
        #     except LinesError as err:
        #         tdSql.checkNotEqual(err.errno, 0)

        # f64
        for t6 in [-1.79769*(10**308), -1.79769*(10**308)]:
            input_json, stb_name = self.genFullTypeJson(tag_value=self.genTagValue(t6_value=t6, value_type=value_type))
            self.resCmp(input_json, stb_name)
        for t6 in [float(-1.797693134862316*(10**308)), -1.797693134862316*(10**308)]:
            input_json = self.genFullTypeJson(tag_value=self.genTagValue(t6_value=t6))[0]
            try:
                self._conn.insert_lines([json.dumps(input_json)], 2)
                raise Exception("should not reach here")
            except LinesError as err:
                tdSql.checkNotEqual(err.errno, 0)

        if value_type == "obj":
            # binary 
            stb_name = tdCom.getLongName(7, "letters")
            input_json = {"metric": stb_name, "timestamp": {'value': 1626006833639000000, 'type': 'ns'}, "value": {'value': True, 'type': 'bool'}, "tags": {"t0": {'value': True, 'type': 'bool'}, "t1":{'value': tdCom.getLongName(16374, "letters"), 'type': 'binary'}}}
            self._conn.insert_lines([json.dumps(input_json)], 2)
            
            input_json = {"metric": stb_name, "timestamp": {'value': 1626006833639000000, 'type': 'ns'}, "value": {'value': True, 'type': 'bool'}, "tags": {"t0": {'value': True, 'type': 'bool'}, "t1":{'value': tdCom.getLongName(16375, "letters"), 'type': 'binary'}}}
            try:
                self._conn.insert_lines([json.dumps(input_json)], 2)
                raise Exception("should not reach here")
            except LinesError as err:
                tdSql.checkNotEqual(err.errno, 0)

            # # nchar
            # # * legal nchar could not be larger than 16374/4
            stb_name = tdCom.getLongName(7, "letters")
            input_json = {"metric": stb_name, "timestamp": {'value': 1626006833639000000, 'type': 'ns'}, "value": {'value': True, 'type': 'bool'}, "tags": {"t0": {'value': True, 'type': 'bool'}, "t1":{'value': tdCom.getLongName(4093, "letters"), 'type': 'nchar'}}}
            self._conn.insert_lines([json.dumps(input_json)], 2)

            input_json = {"metric": stb_name, "timestamp": {'value': 1626006833639000000, 'type': 'ns'}, "value": {'value': True, 'type': 'bool'}, "tags": {"t0": {'value': True, 'type': 'bool'}, "t1":{'value': tdCom.getLongName(4094, "letters"), 'type': 'nchar'}}}
            try:
                self._conn.insert_lines([json.dumps(input_json)], 2)
                raise Exception("should not reach here")
            except LinesError as err:
                tdSql.checkNotEqual(err.errno, 0)
        elif value_type == "default":
            stb_name = tdCom.getLongName(7, "letters")
            if tdSql.getVariable("defaultJSONStrType")[0].lower() == "binary":
                input_json = {"metric": stb_name, "timestamp": 1626006833639019, "value": True, "tags": {"t0": {'value': True, 'type': 'bool'}, "t1": tdCom.getLongName(16374, "letters")}}
            elif tdSql.getVariable("defaultJSONStrType")[0].lower() == "nchar":
                input_json = {"metric": stb_name, "timestamp": 1626006833639019, "value": True, "tags": {"t0": {'value': True, 'type': 'bool'}, "t1": tdCom.getLongName(4093, "letters")}}
            self._conn.insert_lines([json.dumps(input_json)], 2)
            if tdSql.getVariable("defaultJSONStrType")[0].lower() == "binary":
                input_json = {"metric": stb_name, "timestamp": 1626006833639019, "value": True, "tags": {"t0": {'value': True, 'type': 'bool'}, "t1": tdCom.getLongName(16375, "letters")}}
            elif tdSql.getVariable("defaultJSONStrType")[0].lower() == "nchar":
                input_json = {"metric": stb_name, "timestamp": 1626006833639019, "value": True, "tags": {"t0": {'value': True, 'type': 'bool'}, "t1": tdCom.getLongName(4094, "letters")}}
            try:
                self._conn.insert_lines([json.dumps(input_json)], 2)
                raise Exception("should not reach here")
            except LinesError as err:
                tdSql.checkNotEqual(err.errno, 0)

    def colValueLengthCheckCase(self, value_type="obj"):
        """
            check full type col value limit
        """
        tdCom.cleanTb()
        # i8
        for value in [-127, 127]:
            input_json, stb_name = self.genFullTypeJson(col_value=self.genTsColValue(value=value, t_type="tinyint", value_type=value_type))
            self.resCmp(input_json, stb_name)
        tdCom.cleanTb()
        for value in [-128, 128]:
            input_json = self.genFullTypeJson(col_value=self.genTsColValue(value=value, t_type="tinyint"))[0]
            try:
                self._conn.insert_lines([json.dumps(input_json)], 2)
                raise Exception("should not reach here")
            except LinesError as err:
                tdSql.checkNotEqual(err.errno, 0)
        # i16
        tdCom.cleanTb()
        for value in [-32767]:
            input_json, stb_name = self.genFullTypeJson(col_value=self.genTsColValue(value=value, t_type="smallint", value_type=value_type))
            self.resCmp(input_json, stb_name)
        tdCom.cleanTb()
        for value in [-32768, 32768]:
            input_json = self.genFullTypeJson(col_value=self.genTsColValue(value=value, t_type="smallint"))[0]
            try:
                self._conn.insert_lines([json.dumps(input_json)], 2)
                raise Exception("should not reach here")
            except LinesError as err:
                tdSql.checkNotEqual(err.errno, 0)

        # i32
        tdCom.cleanTb()
        for value in [-2147483647]:
            input_json, stb_name = self.genFullTypeJson(col_value=self.genTsColValue(value=value, t_type="int", value_type=value_type))
            self.resCmp(input_json, stb_name)
        tdCom.cleanTb()
        for value in [-2147483648, 2147483648]:
            input_json = self.genFullTypeJson(col_value=self.genTsColValue(value=value, t_type="int"))[0]
            try:
                self._conn.insert_lines([json.dumps(input_json)], 2)
                raise Exception("should not reach here")
            except LinesError as err:
                tdSql.checkNotEqual(err.errno, 0)

        # i64 
        tdCom.cleanTb()
        for value in [-9223372036854775807]:
            input_json, stb_name = self.genFullTypeJson(col_value=self.genTsColValue(value=value, t_type="bigint", value_type=value_type))
            self.resCmp(input_json, stb_name)
        tdCom.cleanTb()
        for value in [-9223372036854775808, 9223372036854775808]:
            input_json = self.genFullTypeJson(col_value=self.genTsColValue(value=value, t_type="bigint", value_type=value_type))[0]
            try:
                self._conn.insert_lines([json.dumps(input_json)], 2)
                raise Exception("should not reach here")
            except LinesError as err:
                tdSql.checkNotEqual(err.errno, 0)

        # f32       
        tdCom.cleanTb()
        for value in [-3.4028234663852885981170418348451692544*(10**38), 3.4028234663852885981170418348451692544*(10**38)]:
            input_json, stb_name = self.genFullTypeJson(col_value=self.genTsColValue(value=value, t_type="float", value_type=value_type))
            self.resCmp(input_json, stb_name)
        # * limit set to 4028234664*(10**38)
        tdCom.cleanTb()
        for value in [-3.4028234664*(10**38), 3.4028234664*(10**38)]:
            input_json = self.genFullTypeJson(col_value=self.genTsColValue(value=value, t_type="float"))[0]
            try:
                self._conn.insert_lines([json.dumps(input_json)], 2)
                raise Exception("should not reach here")
            except LinesError as err:
                tdSql.checkNotEqual(err.errno, 0)

        # f64 #! bug
        # tdCom.cleanTb()
        # for value in [-1.79769313486231570814527423731704356798070567525844996598917476803157260780*(10**308), -1.79769313486231570814527423731704356798070567525844996598917476803157260780*(10**308)]:
        #     input_json, stb_name = self.genFullTypeJson(col_value=self.genTsColValue(value=value, t_type="double", value_type=value_type))
        #     print("value_type----", value_type)
        #     print("input_json----", input_json)
        #     self.resCmp(input_json, stb_name)
        # * limit set to 1.797693134862316*(10**308)
        tdCom.cleanTb()
        for value in [-1.797693134862316*(10**308), -1.797693134862316*(10**308)]:
            input_json = self.genFullTypeJson(col_value=self.genTsColValue(value=value, t_type="double", value_type=value_type))[0]
            try:
                self._conn.insert_lines([json.dumps(input_json)], 2)
                raise Exception("should not reach here")
            except LinesError as err:
                tdSql.checkNotEqual(err.errno, 0)

        if value_type == "obj":
            # binary 
            tdCom.cleanTb()
            stb_name = tdCom.getLongName(7, "letters")
            input_json = {"metric": stb_name, "timestamp":  {'value': 1626006833639000000, 'type': 'ns'}, "value": {'value': tdCom.getLongName(16374, "letters"), 'type': 'binary'}, "tags": {"t0": {'value': True, 'type': 'bool'}}}
            self._conn.insert_lines([json.dumps(input_json)], 2)
            
            tdCom.cleanTb()
            input_json = {"metric": stb_name, "timestamp":  {'value': 1626006833639000000, 'type': 'ns'}, "value": {'value': tdCom.getLongName(16375, "letters"), 'type': 'binary'}, "tags": {"t0": {'value': True, 'type': 'bool'}}}
            try:
                self._conn.insert_lines([json.dumps(input_json)], 2)
                raise Exception("should not reach here")
            except LinesError as err:
                tdSql.checkNotEqual(err.errno, 0)

            # nchar
            # * legal nchar could not be larger than 16374/4
            tdCom.cleanTb()
            stb_name = tdCom.getLongName(7, "letters")
            input_json = {"metric": stb_name, "timestamp":  {'value': 1626006833639000000, 'type': 'ns'}, "value": {'value': tdCom.getLongName(4093, "letters"), 'type': 'nchar'}, "tags": {"t0": {'value': True, 'type': 'bool'}}}
            self._conn.insert_lines([json.dumps(input_json)], 2)

            tdCom.cleanTb()
            input_json = {"metric": stb_name, "timestamp":  {'value': 1626006833639000000, 'type': 'ns'}, "value": {'value': tdCom.getLongName(4094, "letters"), 'type': 'nchar'}, "tags": {"t0": {'value': True, 'type': 'bool'}}}
            try:
                self._conn.insert_lines([json.dumps(input_json)], 2)
                raise Exception("should not reach here")
            except LinesError as err:
                tdSql.checkNotEqual(err.errno, 0)
        elif value_type == "default":
            # binary 
            tdCom.cleanTb()
            stb_name = tdCom.getLongName(7, "letters")
            if tdSql.getVariable("defaultJSONStrType")[0].lower() == "binary":
                input_json = {"metric": stb_name, "timestamp": 1626006833639019, "value": tdCom.getLongName(16374, "letters"), "tags": {"t0": {'value': True, 'type': 'bool'}}}
            elif tdSql.getVariable("defaultJSONStrType")[0].lower() == "nchar":
                input_json = {"metric": stb_name, "timestamp": 1626006833639019, "value": tdCom.getLongName(4093, "letters"), "tags": {"t0": {'value': True, 'type': 'bool'}}}
            self._conn.insert_lines([json.dumps(input_json)], 2)
            tdCom.cleanTb()
            if tdSql.getVariable("defaultJSONStrType")[0].lower() == "binary":
                input_json = {"metric": stb_name, "timestamp": 1626006833639019, "value": tdCom.getLongName(16375, "letters"), "tags": {"t0": {'value': True, 'type': 'bool'}}}
            elif tdSql.getVariable("defaultJSONStrType")[0].lower() == "nchar":
                input_json = {"metric": stb_name, "timestamp": 1626006833639019, "value": tdCom.getLongName(4094, "letters"), "tags": {"t0": {'value': True, 'type': 'bool'}}}
            try:
                self._conn.insert_lines([json.dumps(input_json)], 2)
                raise Exception("should not reach here")
            except LinesError as err:
                tdSql.checkNotEqual(err.errno, 0)

    def tagColIllegalValueCheckCase(self, value_type="obj"):

        """
            test illegal tag col value
        """
        tdCom.cleanTb()
        # bool
        for i in ["TrUe", "tRue", "trUe", "truE", "FalsE", "fAlse", "faLse", "falSe", "falsE"]:
            try:
                input_json1 = self.genFullTypeJson(tag_value=self.genTagValue(t0_value=i))[0]
                self._conn.insert_lines([json.dumps(input_json1)], 2)
                input_json2 = self.genFullTypeJson(col_value=self.genTsColValue(value=i, t_type="bool"))[0]
                self._conn.insert_lines([json.dumps(input_json2)], 2)
            except LinesError as err:
                tdSql.checkNotEqual(err.errno, 0)

        # i8 i16 i32 i64 f32 f64
        for input_json in [
                self.genFullTypeJson(tag_value=self.genTagValue(t1_value="1s2"))[0], 
                self.genFullTypeJson(tag_value=self.genTagValue(t2_value="1s2"))[0], 
                self.genFullTypeJson(tag_value=self.genTagValue(t3_value="1s2"))[0], 
                self.genFullTypeJson(tag_value=self.genTagValue(t4_value="1s2"))[0], 
                self.genFullTypeJson(tag_value=self.genTagValue(t5_value="11.1s45"))[0], 
                self.genFullTypeJson(tag_value=self.genTagValue(t6_value="11.1s45"))[0], 
            ]:
            try:
                self._conn.insert_lines([json.dumps(input_json)], 2)
            except LinesError as err:
                tdSql.checkNotEqual(err.errno, 0)
                

        # check binary and nchar blank
        input_sql1 = self.genFullTypeJson(col_value=self.genTsColValue(value="abc aaa", t_type="binary", value_type=value_type))[0]
        input_sql2 = self.genFullTypeJson(col_value=self.genTsColValue(value="abc aaa", t_type="nchar", value_type=value_type))[0]
        input_sql3 = self.genFullTypeJson(tag_value=self.genTagValue(t7_value="abc aaa", value_type=value_type))[0]
        input_sql4 = self.genFullTypeJson(tag_value=self.genTagValue(t8_value="abc aaa", value_type=value_type))[0]
        for input_json in [input_sql1, input_sql2, input_sql3, input_sql4]:
            try:
                self._conn.insert_lines([json.dumps(input_json)], 2)
            except LinesError as err:
                tdSql.checkNotEqual(err.errno, 0)

        # check accepted binary and nchar symbols 
        # # * ~!@#$¥%^&*()-+={}|[]、「」:;
        for symbol in list('~!@#$¥%^&*()-+={}|[]、「」:;'):
            input_json1 = self.genFullTypeJson(col_value=self.genTsColValue(value=f"abc{symbol}aaa", t_type="binary", value_type=value_type))[0]
            input_json2 = self.genFullTypeJson(tag_value=self.genTagValue(t8_value=f"abc{symbol}aaa", value_type=value_type))[0]
            self._conn.insert_lines([json.dumps(input_json1)], 2)
            self._conn.insert_lines([json.dumps(input_json2)], 2)

    def duplicateIdTagColInsertCheckCase(self):
        """
            check duplicate Id Tag Col
        """
        tdCom.cleanTb()
        input_json = self.genFullTypeJson(id_double_tag=True)[0]
        try:
            self._conn.insert_lines([json.dumps(input_json)], 2)
            raise Exception("should not reach here")
        except LinesError as err:
            tdSql.checkNotEqual(err.errno, 0)

        input_json = self.genFullTypeJson(tag_value=self.genTagValue(t5_value=11.12345027923584, t6_type="float", t6_value=22.12345027923584))[0]
        try:
            self._conn.insert_lines([json.dumps(input_json).replace("t6", "t5")], 2)
            raise Exception("should not reach here")
        except LinesError as err:
            tdSql.checkNotEqual(err.errno, 0)

    ##### stb exist #####
    def noIdStbExistCheckCase(self):
        """
            case no id when stb exist
        """
        tdCom.cleanTb()
        input_json, stb_name = self.genFullTypeJson(tb_name="sub_table_0123456", col_value=self.genTsColValue(value=True, t_type="bool"), tag_value=self.genTagValue(t0_value=True))
        self.resCmp(input_json, stb_name)
        input_json, stb_name = self.genFullTypeJson(stb_name=stb_name, id_noexist_tag=True, col_value=self.genTsColValue(value=True, t_type="bool"), tag_value=self.genTagValue(t0_value=True))
        self.resCmp(input_json, stb_name, condition='where tbname like "t_%"')
        tdSql.query(f"select * from {stb_name}")
        tdSql.checkRows(2)

    def duplicateInsertExistCheckCase(self):
        """
            check duplicate insert when stb exist
        """
        tdCom.cleanTb()
        input_json, stb_name = self.genFullTypeJson()
        self.resCmp(input_json, stb_name)
        self._conn.insert_lines([json.dumps(input_json)], 2)
        self.resCmp(input_json, stb_name)

    def tagColBinaryNcharLengthCheckCase(self):
        """
            check length increase
        """
        tdCom.cleanTb()
        input_json, stb_name = self.genFullTypeJson()
        self._conn.insert_lines([json.dumps(input_json)], 2)
        self.resCmp(input_json, stb_name)
        tb_name = tdCom.getLongName(5, "letters")
        input_json, stb_name = self.genFullTypeJson(stb_name=stb_name, tb_name=tb_name, tag_value=self.genTagValue(t7_value="binaryTagValuebinaryTagValue", t8_value="ncharTagValuencharTagValue"))
        self._conn.insert_lines([json.dumps(input_json)], 2)
        self.resCmp(input_json, stb_name, condition=f'where tbname like "{tb_name}"')

    def lengthIcreaseCrashCheckCase(self):
        """
            check length increase
        """
        tdCom.cleanTb()
        stb_name = "test_crash"
        input_json = self.genFullTypeJson(stb_name=stb_name)[0]
        self._conn.insert_lines([json.dumps(input_json)], 2)
        os.system('python3 query/schemalessQueryCrash.py &')
        time.sleep(10)
        tb_name = tdCom.getLongName(5, "letters")
        input_json, stb_name = self.genFullTypeJson(stb_name=stb_name, tb_name=tb_name, tag_value=self.genTagValue(t7_value="binaryTagValuebinaryTagValue", t8_value="ncharTagValuencharTagValue"))
        print("tag1")
        self._conn.insert_lines([json.dumps(input_json)], 2)
        print("tag2")
        time.sleep(6)
        tdSql.query(f"select * from {stb_name}")
        tdSql.checkRows(2)

    # def tagColBinaryNcharLengthCheckCase(self):
    #     """
    #         check length increase
    #     """
    #     tdCom.cleanTb()
    #     stb_name = "test_crash"
    #     tb_name = "test_crash_1"
    #     input_json = self.genFullTypeJson(stb_name=stb_name)[0]
    #     print(input_json)
        self._conn.insert_lines([json.dumps(input_json)], 2)
        time.sleep(20)
        # self.resCmp(input_json, stb_name)
        tb_name = tdCom.getLongName(5, "letters")
        input_json, stb_name = self.genFullTypeJson(stb_name=stb_name, tb_name=tb_name, tag_value=self.genTagValue(t7_value="binaryTagValuebinaryTagValue", t8_value="ncharTagValuencharTagValue"))
        self._conn.insert_lines([json.dumps(input_json)], 2)

    #     # self.resCmp(input_json, stb_name, condition=f'where tbname like "{tb_name}"')

    def tagColAddDupIDCheckCase(self):
        """
            check tag count add, stb and tb duplicate
            * tag: alter table ...
            * col: when update==0 and ts is same, unchange
            * so this case tag&&value will be added, 
            * col is added without value when update==0
            * col is added with value when update==1
        """
        tdCom.cleanTb()
        tb_name = tdCom.getLongName(7, "letters")
        for db_update_tag in [0, 1]:
            if db_update_tag == 1 :
                self.createDb("test_update", db_update_tag=db_update_tag)
            input_json, stb_name = self.genFullTypeJson(tb_name=tb_name, col_value=self.genTsColValue(value=True, t_type="bool"), tag_value=self.genTagValue(t0_value=True))
            self.resCmp(input_json, stb_name)
            self.genFullTypeJson(stb_name=stb_name, tb_name=tb_name, col_value=self.genTsColValue(value=True, t_type="bool"), tag_value=self.genTagValue(t0_value=True), t_add_tag=True)
            if db_update_tag == 1 :
                self.resCmp(input_json, stb_name, condition=f'where tbname like "{tb_name}"')
            else:
                self.resCmp(input_json, stb_name, condition=f'where tbname like "{tb_name}"', none_check_tag=True)
            self.createDb()

    def tagColAddCheckCase(self):
        """
            check tag count add
        """
        tdCom.cleanTb()
        tb_name = tdCom.getLongName(7, "letters")
        input_json, stb_name = self.genFullTypeJson(tb_name=tb_name, col_value=self.genTsColValue(value=True, t_type="bool"), tag_value=self.genTagValue(t0_value=True))
        self.resCmp(input_json, stb_name)
        tb_name_1 = tdCom.getLongName(7, "letters")
        input_json, stb_name = self.genFullTypeJson(stb_name=stb_name, tb_name=tb_name_1, col_value=self.genTsColValue(value=True, t_type="bool"), tag_value=self.genTagValue(t0_value=True), t_add_tag=True)
        self.resCmp(input_json, stb_name, condition=f'where tbname like "{tb_name_1}"')
        res_row_list = self.resHandle(f"select t10,t11 from {tb_name}", True)[0]
        tdSql.checkEqual(res_row_list[0], ['None', 'None'])
        self.resCmp(input_json, stb_name, condition=f'where tbname like "{tb_name}"', none_check_tag=True)

    def tagMd5Check(self):
        """
            condition: stb not change
            insert two table, keep tag unchange, change col
        """
        tdCom.cleanTb()
        input_json, stb_name = self.genFullTypeJson(col_value=self.genTsColValue(value=True, t_type="bool"), tag_value=self.genTagValue(t0_value=True), id_noexist_tag=True)
        self.resCmp(input_json, stb_name)
        tb_name1 = self.getNoIdTbName(stb_name)
        input_json, stb_name = self.genFullTypeJson(stb_name=stb_name, col_value=self.genTsColValue(value=True, t_type="bool"), tag_value=self.genTagValue(t0_value=True), id_noexist_tag=True)
        self.resCmp(input_json, stb_name)
        tb_name2 = self.getNoIdTbName(stb_name)
        tdSql.query(f"select * from {stb_name}")
        tdSql.checkRows(1)
        tdSql.checkEqual(tb_name1, tb_name2)
        input_json, stb_name = self.genFullTypeJson(stb_name=stb_name, col_value=self.genTsColValue(value=True, t_type="bool"), tag_value=self.genTagValue(t0_value=True), id_noexist_tag=True, t_add_tag=True)
        self._conn.insert_lines([json.dumps(input_json)], 2)
        tb_name3 = self.getNoIdTbName(stb_name)
        tdSql.query(f"select * from {stb_name}")
        tdSql.checkRows(2)
        tdSql.checkNotEqual(tb_name1, tb_name3)

    # * tag binary max is 16384, col+ts binary max  49151
    def tagColBinaryMaxLengthCheckCase(self):
        """
            every binary and nchar must be length+2
        """
        tdCom.cleanTb()
        stb_name = tdCom.getLongName(7, "letters")
        tb_name = f'{stb_name}_1'
        tag_value = {"t0": {"value": True, "type": "bool"}}
        tag_value["id"] = tb_name
        col_value=self.genTsColValue(value=True, t_type="bool")
        input_json = {"metric": stb_name, "timestamp": {"value": 1626006833639000000, "type": "ns"}, "value": col_value, "tags": tag_value}
        self._conn.insert_lines([json.dumps(input_json)], 2)

        # * every binary and nchar must be length+2, so here is two tag, max length could not larger than 16384-2*2
        tag_value["t1"] = {"value": tdCom.getLongName(16374, "letters"), "type": "binary"}
        tag_value["t2"] = {"value": tdCom.getLongName(5, "letters"), "type": "binary"}
        tag_value.pop('id')
        self._conn.insert_lines([json.dumps(input_json)], 2)
        
        tdSql.query(f"select * from {stb_name}")
        tdSql.checkRows(2)
        tag_value["t2"] = {"value": tdCom.getLongName(6, "letters"), "type": "binary"}
        try:
            self._conn.insert_lines([json.dumps(input_json)], 2)
            raise Exception("should not reach here")
        except LinesError as err:
            tdSql.checkNotEqual(err.errno, 0)
        tdSql.query(f"select * from {stb_name}")
        tdSql.checkRows(2)

    # * tag nchar max is 16374/4, col+ts nchar max  49151
    def tagColNcharMaxLengthCheckCase(self):
        """
            check nchar length limit
        """
        tdCom.cleanTb()
        stb_name = tdCom.getLongName(7, "letters")
        tb_name = f'{stb_name}_1'
        tag_value = {"t0": {"value": True, "type": "bool"}}
        tag_value["id"] = tb_name
        col_value=self.genTsColValue(value=True, t_type="bool")
        input_json = {"metric": stb_name, "timestamp": {"value": 1626006833639000000, "type": "ns"}, "value": col_value, "tags": tag_value}
        self._conn.insert_lines([json.dumps(input_json)], 2)

        # * legal nchar could not be larger than 16374/4
        tag_value["t1"] = {"value": tdCom.getLongName(4093, "letters"), "type": "nchar"}
        tag_value["t2"] = {"value": tdCom.getLongName(1, "letters"), "type": "nchar"}
        tag_value.pop('id')
        self._conn.insert_lines([json.dumps(input_json)], 2)
        tdSql.query(f"select * from {stb_name}")
        tdSql.checkRows(2)
        tag_value["t2"] = {"value": tdCom.getLongName(2, "letters"), "type": "binary"}
        try:
            self._conn.insert_lines([json.dumps(input_json)], 2)
            raise Exception("should not reach here")
        except LinesError as err:
            tdSql.checkNotEqual(err.errno, 0)
        tdSql.query(f"select * from {stb_name}")
        tdSql.checkRows(2)

    def batchInsertCheckCase(self):
        """
            test batch insert
        """
        tdCom.cleanTb()
        stb_name = tdCom.getLongName(8, "letters")
        tdSql.execute(f'create stable {stb_name}(ts timestamp, f int) tags(t1 bigint)')
        input_json = [{"metric": "st123456", "timestamp": {"value": 1626006833639000000, "type": "ns"}, "value": {"value": 1, "type": "bigint"}, "tags": {"t1": {"value": 3, "type": "bigint"}, "t2": {"value": 4, "type": "double"}, "t3": {"value": "t3", "type": "binary"}}},
                    {"metric": "st123456", "timestamp": {"value": 1626006833640000000, "type": "ns"}, "value": {"value": 2, "type": "bigint"}, "tags": {"t1": {"value": 4, "type": "bigint"}, "t3": {"value": "t4", "type": "binary"}, "t2": {"value": 5, "type": "double"}, "t4": {"value": 5, "type": "double"}}},
                    {"metric": stb_name, "timestamp": {"value": 1626056811823316532, "type": "ns"}, "value": {"value": 3, "type": "bigint"}, "tags": {"t2": {"value": 5, "type": "double"}, "t3": {"value": "ste", "type": "nchar"}}},
                    {"metric": "stf567890", "timestamp": {"value": 1626006933640000000, "type": "ns"}, "value": {"value": 4, "type": "bigint"}, "tags": {"t1": {"value": 4, "type": "bigint"}, "t3": {"value": "t4", "type": "binary"}, "t2": {"value": 5, "type": "double"}, "t4": {"value": 5, "type": "double"}}},
                    {"metric": "st123456", "timestamp": {"value": 1626006833642000000, "type": "ns"}, "value": {"value": 5, "type": "bigint"}, "tags": {"t1": {"value": 4, "type": "bigint"}, "t2": {"value": 5, "type": "double"}, "t3": {"value": "t4", "type": "binary"}}},
                    {"metric": stb_name, "timestamp": {"value": 1626056811843316532, "type": "ns"}, "value": {"value": 6, "type": "bigint"}, "tags": {"t2": {"value": 5, "type": "double"}, "t3": {"value": "ste2", "type": "nchar"}}},
                    {"metric": stb_name, "timestamp": {"value": 1626056812843316532, "type": "ns"}, "value": {"value": 7, "type": "bigint"}, "tags": {"t2": {"value": 5, "type": "double"}, "t3": {"value": "ste2", "type": "nchar"}}},
                    {"metric": "st123456", "timestamp": {"value": 1626006933640000000, "type": "ns"}, "value": {"value": 8, "type": "bigint"}, "tags": {"t1": {"value": 4, "type": "bigint"}, "t3": {"value": "t4", "type": "binary"}, "t2": {"value": 5, "type": "double"}, "t4": {"value": 5, "type": "double"}}},
                    {"metric": "st123456", "timestamp": {"value": 1626006933641000000, "type": "ns"}, "value": {"value": 9, "type": "bigint"}, "tags": {"t1": {"value": 4, "type": "bigint"}, "t3": {"value": "t4", "type": "binary"}, "t2": {"value": 5, "type": "double"}, "t4": {"value": 5, "type": "double"}}}]
        self._conn.insert_lines([json.dumps(input_json)], 2)
        tdSql.query('show stables')
        tdSql.checkRows(3)
        tdSql.query('show tables')
        tdSql.checkRows(6)
        tdSql.query('select * from st123456')
        tdSql.checkRows(5)
    
    def multiInsertCheckCase(self, count):
            """
                test multi insert
            """
            tdCom.cleanTb()
            sql_list = list()
            stb_name = tdCom.getLongName(8, "letters")
            tdSql.execute(f'create stable {stb_name}(ts timestamp, f int) tags(t1 bigint)')
            for i in range(count):
                input_json = self.genFullTypeJson(stb_name=stb_name, col_value=self.genTsColValue(value=tdCom.getLongName(8, "letters"), t_type="binary"), tag_value=self.genTagValue(t7_value=tdCom.getLongName(8, "letters")), id_noexist_tag=True)[0]
                sql_list.append(input_json)
            self._conn.insert_lines([json.dumps(sql_list)], 2)
            tdSql.query('show tables')
            tdSql.checkRows(count)

    def batchErrorInsertCheckCase(self):
        """
            test batch error insert
        """
        tdCom.cleanTb()
        input_json = [{"metric": "st123456", "timestamp": {"value": 1626006833639000000, "type": "ns"}, "value": {"value": "tt", "type": "bool"}, "tags": {"t1": {"value": 3, "type": "bigint"}, "t2": {"value": 4, "type": "double"}, "t3": {"value": "t3", "type": "binary"}}},
                    {"metric": "st123456", "timestamp": {"value": 1626006933641000000, "type": "ns"}, "value": {"value": 9, "type": "bigint"}, "tags": {"t1": {"value": 4, "type": "bigint"}, "t3": {"value": "t4", "type": "binary"}, "t2": {"value": 5, "type": "double"}, "t4": {"value": 5, "type": "double"}}}]
        try:
            self._conn.insert_lines([json.dumps(input_json)], 2)
            raise Exception("should not reach here")
        except LinesError as err:
            tdSql.checkNotEqual(err.errno, 0)

    def multiColsInsertCheckCase(self):
        """
            test multi cols insert
        """
        tdCom.cleanTb()
        input_json = self.genFullTypeJson(c_multi_tag=True)[0]
        try:
            self._conn.insert_lines([json.dumps(input_json)], 2)
            raise Exception("should not reach here")
        except LinesError as err:
            tdSql.checkNotEqual(err.errno, 0)
    
    def blankColInsertCheckCase(self):
        """
            test blank col insert
        """
        tdCom.cleanTb()
        input_json = self.genFullTypeJson(c_blank_tag=True)[0]
        try:
            self._conn.insert_lines([json.dumps(input_json)], 2)
            raise Exception("should not reach here")
        except LinesError as err:
            tdSql.checkNotEqual(err.errno, 0)

    def blankTagInsertCheckCase(self):
        """
            test blank tag insert
        """
        tdCom.cleanTb()
        input_json = self.genFullTypeJson(t_blank_tag=True)[0]
        try:
            self._conn.insert_lines([json.dumps(input_json)], 2)
            raise Exception("should not reach here")
        except LinesError as err:
            tdSql.checkNotEqual(err.errno, 0)
    
    def chineseCheckCase(self):
        """
            check nchar ---> chinese
        """
        tdCom.cleanTb()
        input_json, stb_name = self.genFullTypeJson(chinese_tag=True)
        self.resCmp(input_json, stb_name)

    def multiFieldCheckCase(self):
        '''
            multi_field
        '''
        tdCom.cleanTb()
        input_json = self.genFullTypeJson(multi_field_tag=True)[0]
        try:
            self._conn.insert_lines([json.dumps(input_json)], 2)
            raise Exception("should not reach here")
        except LinesError as err:
            tdSql.checkNotEqual(err.errno, 0)

    def spellCheckCase(self):
        stb_name = tdCom.getLongName(8, "letters")
        input_json_list = [{"metric": f'{stb_name}_1', "timestamp": {"value": 1626006833639000000, "type": "Ns"}, "value": {"value": 1, "type": "Bigint"}, "tags": {"t1": {"value": 127, "type": "tinYint"}}},
                        {"metric": f'{stb_name}_2', "timestamp": {"value": 1626006833639000001, "type": "nS"}, "value": {"value": 32767, "type": "smallInt"}, "tags": {"t1": {"value": 32767, "type": "smallInt"}}},
                        {"metric": f'{stb_name}_3', "timestamp": {"value": 1626006833639000002, "type": "NS"}, "value": {"value": 2147483647, "type": "iNt"}, "tags": {"t1": {"value": 2147483647, "type": "iNt"}}},
                        {"metric": f'{stb_name}_4', "timestamp": {"value": 1626006833639019, "type": "Us"}, "value": {"value": 9223372036854775807, "type": "bigInt"}, "tags": {"t1": {"value": 9223372036854775807, "type": "bigInt"}}},
                        {"metric": f'{stb_name}_5', "timestamp": {"value": 1626006833639018, "type": "uS"}, "value": {"value": 11.12345027923584, "type": "flOat"}, "tags": {"t1": {"value": 11.12345027923584, "type": "flOat"}}},
                        {"metric": f'{stb_name}_6', "timestamp": {"value": 1626006833639017, "type": "US"}, "value": {"value": 22.123456789, "type": "douBle"}, "tags": {"t1": {"value": 22.123456789, "type": "douBle"}}},
                        {"metric": f'{stb_name}_7', "timestamp": {"value": 1626006833640, "type": "Ms"}, "value": {"value": "vozamcts", "type": "binaRy"}, "tags": {"t1": {"value": "vozamcts", "type": "binaRy"}}},
                        {"metric": f'{stb_name}_8', "timestamp": {"value": 1626006833641, "type": "mS"}, "value": {"value": "vozamcts", "type": "nchAr"}, "tags": {"t1": {"value": "vozamcts", "type": "nchAr"}}},
                        {"metric": f'{stb_name}_9', "timestamp": {"value": 1626006833642, "type": "MS"}, "value": {"value": "vozamcts", "type": "nchAr"}, "tags": {"t1": {"value": "vozamcts", "type": "nchAr"}}},
                        {"metric": f'{stb_name}_10', "timestamp": {"value": 1626006834, "type": "S"}, "value": {"value": "vozamcts", "type": "nchAr"}, "tags": {"t1": {"value": "vozamcts", "type": "nchAr"}}}]
       
        for input_sql in input_json_list:
            stb_name = input_sql["metric"]
            self.resCmp(input_sql, stb_name)

    def pointTransCheckCase(self):
        """
            metric value "." trans to "_"
        """
        tdCom.cleanTb()
        input_json = self.genFullTypeJson(point_trans_tag=True)[0]
        stb_name = input_json["metric"].replace(".", "_")
        self.resCmp(input_json, stb_name)

    def genSqlList(self, count=5, stb_name="", tb_name=""):
        """
            stb --> supertable
            tb  --> table
            ts  --> timestamp, same default
            col --> column, same default
            tag --> tag, same default
            d   --> different
            s   --> same
            a   --> add
            m   --> minus
        """
        d_stb_d_tb_list = list()
        s_stb_s_tb_list = list()
        s_stb_s_tb_a_tag_list = list()
        s_stb_s_tb_m_tag_list = list()
        s_stb_d_tb_list = list()
        s_stb_d_tb_m_tag_list = list()
        s_stb_d_tb_a_tag_list = list()
        s_stb_s_tb_d_ts_list = list()
        s_stb_s_tb_d_ts_m_tag_list = list()
        s_stb_s_tb_d_ts_a_tag_list = list()
        s_stb_d_tb_d_ts_list = list()
        s_stb_d_tb_d_ts_m_tag_list = list()
        s_stb_d_tb_d_ts_a_tag_list = list()
        for i in range(count):
            d_stb_d_tb_list.append(self.genFullTypeJson(col_value=self.genTsColValue(value=True, t_type="bool"), tag_value=self.genTagValue(t0_value=True)))
            s_stb_s_tb_list.append(self.genFullTypeJson(stb_name=stb_name, tb_name=tb_name, col_value=self.genTsColValue(value=tdCom.getLongName(8, "letters"), t_type="binary"), tag_value=self.genTagValue(t7_value=tdCom.getLongName(8, "letters"))))
            s_stb_s_tb_a_tag_list.append(self.genFullTypeJson(stb_name=stb_name, tb_name=tb_name, col_value=self.genTsColValue(value=tdCom.getLongName(8, "letters"), t_type="binary"), tag_value=self.genTagValue(t7_value=tdCom.getLongName(8, "letters")), t_add_tag=True))
            s_stb_s_tb_m_tag_list.append(self.genFullTypeJson(stb_name=stb_name, tb_name=tb_name, col_value=self.genTsColValue(value=tdCom.getLongName(8, "letters"), t_type="binary"), tag_value=self.genTagValue(t7_value=tdCom.getLongName(8, "letters")), t_mul_tag=True))
            s_stb_d_tb_list.append(self.genFullTypeJson(stb_name=stb_name, col_value=self.genTsColValue(value=tdCom.getLongName(8, "letters"), t_type="binary"), tag_value=self.genTagValue(t7_value=tdCom.getLongName(8, "letters")), id_noexist_tag=True))
            s_stb_d_tb_m_tag_list.append(self.genFullTypeJson(stb_name=stb_name, col_value=self.genTsColValue(value=tdCom.getLongName(8, "letters"), t_type="binary"), tag_value=self.genTagValue(t7_value=tdCom.getLongName(8, "letters")), id_noexist_tag=True, t_mul_tag=True))
            s_stb_d_tb_a_tag_list.append(self.genFullTypeJson(stb_name=stb_name, col_value=self.genTsColValue(value=tdCom.getLongName(8, "letters"), t_type="binary"), tag_value=self.genTagValue(t7_value=tdCom.getLongName(8, "letters")), id_noexist_tag=True, t_add_tag=True))
            s_stb_s_tb_d_ts_list.append(self.genFullTypeJson(stb_name=stb_name, tb_name=tb_name, col_value=self.genTsColValue(value=tdCom.getLongName(8, "letters"), t_type="binary"), tag_value=self.genTagValue(t7_value=tdCom.getLongName(8, "letters")), ts_value = self.genTsColValue(1626006833639000000, "ns")))
            s_stb_s_tb_d_ts_m_tag_list.append(self.genFullTypeJson(stb_name=stb_name, tb_name=tb_name, col_value=self.genTsColValue(value=tdCom.getLongName(8, "letters"), t_type="binary"), tag_value=self.genTagValue(t7_value=tdCom.getLongName(8, "letters")), ts_value = self.genTsColValue(1626006833639000000, "ns"), t_mul_tag=True))
            s_stb_s_tb_d_ts_a_tag_list.append(self.genFullTypeJson(stb_name=stb_name, tb_name=tb_name, col_value=self.genTsColValue(value=tdCom.getLongName(8, "letters"), t_type="binary"), tag_value=self.genTagValue(t7_value=tdCom.getLongName(8, "letters")), ts_value = self.genTsColValue(1626006833639000000, "ns"), t_add_tag=True))
            s_stb_d_tb_d_ts_list.append(self.genFullTypeJson(stb_name=stb_name, col_value=self.genTsColValue(value=tdCom.getLongName(8, "letters"), t_type="binary"), tag_value=self.genTagValue(t7_value=tdCom.getLongName(8, "letters")), id_noexist_tag=True, ts_value = self.genTsColValue(1626006833639000000, "ns")))
            s_stb_d_tb_d_ts_m_tag_list.append(self.genFullTypeJson(stb_name=stb_name, col_value=self.genTsColValue(value=tdCom.getLongName(8, "letters"), t_type="binary"), tag_value=self.genTagValue(t7_value=tdCom.getLongName(8, "letters")), id_noexist_tag=True, ts_value = self.genTsColValue(0, "ns"), t_mul_tag=True))
            s_stb_d_tb_d_ts_a_tag_list.append(self.genFullTypeJson(stb_name=stb_name, col_value=self.genTsColValue(value=tdCom.getLongName(8, "letters"), t_type="binary"), tag_value=self.genTagValue(t7_value=tdCom.getLongName(8, "letters")), id_noexist_tag=True, ts_value = self.genTsColValue(0, "ns"), t_add_tag=True))

        return d_stb_d_tb_list, s_stb_s_tb_list, s_stb_s_tb_a_tag_list, s_stb_s_tb_m_tag_list, \
            s_stb_d_tb_list, s_stb_d_tb_m_tag_list, s_stb_d_tb_a_tag_list, s_stb_s_tb_d_ts_list, \
            s_stb_s_tb_d_ts_m_tag_list, s_stb_s_tb_d_ts_a_tag_list, s_stb_d_tb_d_ts_list, \
            s_stb_d_tb_d_ts_m_tag_list, s_stb_d_tb_d_ts_a_tag_list


    def genMultiThreadSeq(self, sql_list):
        tlist = list()
        for insert_sql in sql_list:
            t = threading.Thread(target=self._conn.insert_lines,args=([json.dumps(insert_sql[0])], 2))
            tlist.append(t)
        return tlist

    def multiThreadRun(self, tlist):
        for t in tlist:
            t.start()
        for t in tlist:
            t.join()

    def stbInsertMultiThreadCheckCase(self):
        """
            thread input different stb
        """
        tdCom.cleanTb()
        input_json = self.genSqlList()[0]
        self.multiThreadRun(self.genMultiThreadSeq(input_json))
        tdSql.query(f"show tables;")
        tdSql.checkRows(5)
    
    def sStbStbDdataInsertMultiThreadCheckCase(self):
        """
            thread input same stb tb, different data, result keep first data
        """
        tdCom.cleanTb()
        tb_name = tdCom.getLongName(7, "letters")
        input_json, stb_name = self.genFullTypeJson(tb_name=tb_name, col_value=self.genTsColValue(value="binaryTagValue", t_type="binary"))
        self.resCmp(input_json, stb_name)
        s_stb_s_tb_list = self.genSqlList(stb_name=stb_name, tb_name=tb_name)[1]
        self.multiThreadRun(self.genMultiThreadSeq(s_stb_s_tb_list))
        tdSql.query(f"show tables;")
        tdSql.checkRows(1)
        expected_tb_name = self.getNoIdTbName(stb_name)[0]
        tdSql.checkEqual(tb_name, expected_tb_name)
        tdSql.query(f"select * from {stb_name};")
        tdSql.checkRows(1)

    def sStbStbDdataAtInsertMultiThreadCheckCase(self):
        """
            thread input same stb tb, different data, add columes and tags,  result keep first data
        """
        tdCom.cleanTb()
        tb_name = tdCom.getLongName(7, "letters")
        input_json, stb_name = self.genFullTypeJson(tb_name=tb_name, col_value=self.genTsColValue(value="binaryTagValue", t_type="binary"))
        self.resCmp(input_json, stb_name)
        s_stb_s_tb_a_tag_list = self.genSqlList(stb_name=stb_name, tb_name=tb_name)[2]
        self.multiThreadRun(self.genMultiThreadSeq(s_stb_s_tb_a_tag_list))
        tdSql.query(f"show tables;")
        tdSql.checkRows(1)
        expected_tb_name = self.getNoIdTbName(stb_name)[0]
        tdSql.checkEqual(tb_name, expected_tb_name)
        tdSql.query(f"select * from {stb_name};")
        tdSql.checkRows(1)
    
    def sStbStbDdataMtInsertMultiThreadCheckCase(self):
        """
            thread input same stb tb, different data, minus columes and tags,  result keep first data
        """
        tdCom.cleanTb()
        tb_name = tdCom.getLongName(7, "letters")
        input_json, stb_name = self.genFullTypeJson(tb_name=tb_name, col_value=self.genTsColValue(value="binaryTagValue", t_type="binary"))
        self.resCmp(input_json, stb_name)
        s_stb_s_tb_m_tag_list = self.genSqlList(stb_name=stb_name, tb_name=tb_name)[3]
        self.multiThreadRun(self.genMultiThreadSeq(s_stb_s_tb_m_tag_list))
        tdSql.query(f"show tables;")
        tdSql.checkRows(1)
        expected_tb_name = self.getNoIdTbName(stb_name)[0]
        tdSql.checkEqual(tb_name, expected_tb_name)
        tdSql.query(f"select * from {stb_name};")
        tdSql.checkRows(1)

    def sStbDtbDdataInsertMultiThreadCheckCase(self):
        """
            thread input same stb, different tb, different data
        """
        tdCom.cleanTb()
        input_json, stb_name = self.genFullTypeJson(col_value=self.genTsColValue(value="binaryTagValue", t_type="binary"))
        self.resCmp(input_json, stb_name)
        s_stb_d_tb_list = self.genSqlList(stb_name=stb_name)[4]
        self.multiThreadRun(self.genMultiThreadSeq(s_stb_d_tb_list))
        tdSql.query(f"show tables;")
        tdSql.checkRows(6)

    def sStbDtbDdataMtInsertMultiThreadCheckCase(self):
        """
            thread input same stb, different tb, different data, add col, mul tag
        """
        tdCom.cleanTb()
        input_json, stb_name = self.genFullTypeJson(col_value=self.genTsColValue(value="binaryTagValue", t_type="binary"))
        self.resCmp(input_json, stb_name)
        s_stb_d_tb_m_tag_list = [({"metric": stb_name, "timestamp": {"value": 1626006833639000000, "type": "ns"}, "value": {"value": "omfdhyom", "type": "binary"}, "tags": {"t0": {"value": False, "type": "bool"}, "t1": {"value": 127, "type": "tinyint"}, "t2": {"value": 32767, "type": "smallint"}, "t3": {"value": 2147483647, "type": "int"}, "t4": {"value": 9223372036854775807, "type": "bigint"}, "t5": {"value": 11.12345, "type": "float"}, "t6": {"value": 22.123456789, "type": "double"}}}, 'yzwswz'),
                                ({"metric": stb_name, "timestamp": {"value": 1626006833639000000, "type": "ns"}, "value": {"value": "vqowydbc", "type": "binary"}, "tags": {"t0": {"value": False, "type": "bool"}, "t1": {"value": 127, "type": "tinyint"}, "t2": {"value": 32767, "type": "smallint"}, "t3": {"value": 2147483647, "type": "int"}, "t4": {"value": 9223372036854775807, "type": "bigint"}, "t5": {"value": 11.12345, "type": "float"}, "t6": {"value": 22.123456789, "type": "double"}}}, 'yzwswz'),
                                ({"metric": stb_name, "timestamp": {"value": 1626006833639000000, "type": "ns"}, "value": {"value": "plgkckpv", "type": "binary"}, "tags": {"t0": {"value": False, "type": "bool"}, "t1": {"value": 127, "type": "tinyint"}, "t2": {"value": 32767, "type": "smallint"}, "t3": {"value": 2147483647, "type": "int"}, "t4": {"value": 9223372036854775807, "type": "bigint"}, "t5": {"value": 11.12345, "type": "float"}, "t6": {"value": 22.123456789, "type": "double"}}}, 'yzwswz'),
                                ({"metric": stb_name, "timestamp": {"value": 1626006833639000000, "type": "ns"}, "value": {"value": "cujyqvlj", "type": "binary"}, "tags": {"t0": {"value": False, "type": "bool"}, "t1": {"value": 127, "type": "tinyint"}, "t2": {"value": 32767, "type": "smallint"}, "t3": {"value": 2147483647, "type": "int"}, "t4": {"value": 9223372036854775807, "type": "bigint"}, "t5": {"value": 11.12345, "type": "float"}, "t6": {"value": 22.123456789, "type": "double"}}}, 'yzwswz'),
                                ({"metric": stb_name, "timestamp": {"value": 1626006833639000000, "type": "ns"}, "value": {"value": "twjxisat", "type": "binary"}, "tags": {"t0": {"value": False, "type": "bool"}, "t1": {"value": 127, "type": "tinyint"}, "t2": {"value": 32767, "type": "smallint"}, "t3": {"value": 2147483647, "type": "int"}, "t4": {"value": 9223372036854775807, "type": "bigint"}, "t5": {"value": 11.12345, "type": "float"}, "t6": {"value": 22.123456789, "type": "double"}}}, 'yzwswz')]
       
        self.multiThreadRun(self.genMultiThreadSeq(s_stb_d_tb_m_tag_list))
        tdSql.query(f"show tables;")
        tdSql.checkRows(2)

    def sStbDtbDdataAtInsertMultiThreadCheckCase(self):
        """
            thread input same stb, different tb, different data, add tag, mul col
        """
        tdCom.cleanTb()
        input_json, stb_name = self.genFullTypeJson(col_value=self.genTsColValue(value="binaryTagValue", t_type="binary"))
        self.resCmp(input_json, stb_name)
        s_stb_d_tb_a_tag_list = self.genSqlList(stb_name=stb_name)[6]
        self.multiThreadRun(self.genMultiThreadSeq(s_stb_d_tb_a_tag_list))
        tdSql.query(f"show tables;")
        tdSql.checkRows(6)

    def sStbStbDdataDtsInsertMultiThreadCheckCase(self):
        """
            thread input same stb tb, different ts
        """
        tdCom.cleanTb()
        tb_name = tdCom.getLongName(7, "letters")
        input_json, stb_name = self.genFullTypeJson(tb_name=tb_name, col_value=self.genTsColValue(value="binaryTagValue", t_type="binary"))
        self.resCmp(input_json, stb_name)
        s_stb_s_tb_d_ts_list = [({"metric": stb_name, "timestamp": {"value": 0, "type": "ns"}, "value": {"value": "hkgjiwdj", "type": "binary"}, "tags": {"id": tb_name, "t0": {"value": False, "type": "bool"}, "t1": {"value": 127, "type": "tinyint"}, "t2": {"value": 32767, "type": "smallint"}, "t3": {"value": 2147483647, "type": "int"}, "t4": {"value": 9223372036854775807, "type": "bigint"}, "t5": {"value": 11.12345, "type": "float"}, "t6": {"value": 22.123456789, "type": "double"}, "t7": {"value": "vozamcts", "type": "binary"}, "t8": {"value": "ncharTagValue", "type": "nchar"}}}, 'yzwswz'),
                                ({"metric": stb_name, "timestamp": {"value": 0, "type": "ns"}, "value": {"value": "rljjrrul", "type": "binary"}, "tags": {"id": tb_name, "t0": {"value": False, "type": "bool"}, "t1": {"value": 127, "type": "tinyint"}, "t2": {"value": 32767, "type": "smallint"}, "t3": {"value": 2147483647, "type": "int"}, "t4": {"value": 9223372036854775807, "type": "bigint"}, "t5": {"value": 11.12345, "type": "float"}, "t6": {"value": 22.123456789, "type": "double"}, "t7": {"value": "bmcanhbs", "type": "binary"}, "t8": {"value": "ncharTagValue", "type": "nchar"}}}, 'yzwswz'),
                                ({"metric": stb_name, "timestamp": {"value": 0, "type": "ns"}, "value": {"value": "basanglx", "type": "binary"}, "tags": {"id": tb_name, "t0": {"value": False, "type": "bool"}, "t1": {"value": 127, "type": "tinyint"}, "t2": {"value": 32767, "type": "smallint"}, "t3": {"value": 2147483647, "type": "int"}, "t4": {"value": 9223372036854775807, "type": "bigint"}, "t5": {"value": 11.12345, "type": "float"}, "t6": {"value": 22.123456789, "type": "double"}, "t7": {"value": "enqkyvmb", "type": "binary"}, "t8": {"value": "ncharTagValue", "type": "nchar"}}}, 'yzwswz'),
                                ({"metric": stb_name, "timestamp": {"value": 0, "type": "ns"}, "value": {"value": "clsajzpp", "type": "binary"}, "tags": {"id": tb_name, "t0": {"value": False, "type": "bool"}, "t1": {"value": 127, "type": "tinyint"}, "t2": {"value": 32767, "type": "smallint"}, "t3": {"value": 2147483647, "type": "int"}, "t4": {"value": 9223372036854775807, "type": "bigint"}, "t5": {"value": 11.12345, "type": "float"}, "t6": {"value": 22.123456789, "type": "double"}, "t7": {"value": "eivaegjk", "type": "binary"}, "t8": {"value": "ncharTagValue", "type": "nchar"}}}, 'yzwswz'),
                                ({"metric": stb_name, "timestamp": {"value": 0, "type": "ns"}, "value": {"value": "jitwseso", "type": "binary"}, "tags": {"id": tb_name, "t0": {"value": True, "type": "bool"}, "t1": {"value": 127, "type": "tinyint"}, "t2": {"value": 32767, "type": "smallint"}, "t3": {"value": 2147483647, "type": "int"}, "t4": {"value": 9223372036854775807, "type": "bigint"}, "t5": {"value": 11.12345, "type": "float"}, "t6": {"value": 22.123456789, "type": "double"}, "t7": {"value": "yhlwkddq", "type": "binary"}, "t8": {"value": "ncharTagValue", "type": "nchar"}}}, 'yzwswz')]
        self.multiThreadRun(self.genMultiThreadSeq(s_stb_s_tb_d_ts_list))
        tdSql.query(f"show tables;")
        tdSql.checkRows(1)
        tdSql.query(f"select * from {stb_name}")
        tdSql.checkRows(6)

    def sStbStbDdataDtsMtInsertMultiThreadCheckCase(self):
        """
            thread input same stb tb, different ts, add col, mul tag
        """
        tdCom.cleanTb()
        tb_name = tdCom.getLongName(7, "letters")
        input_json, stb_name = self.genFullTypeJson(tb_name=tb_name, col_value=self.genTsColValue(value="binaryTagValue", t_type="binary"))
        self.resCmp(input_json, stb_name)
        s_stb_s_tb_d_ts_m_tag_list = [({'metric': stb_name, 'timestamp': {'value': 0, 'type': 'ns'}, 'value': {'value': 'pjndapjb', 'type': 'binary'}, 'tags': {'t0': {'value': False, 'type': 'bool'}, 't1': {'value': 127, 'type': 'tinyint'}, 't2': {'value': 32767, 'type': 'smallint'}, 't3': {'value': 2147483647, 'type': 'int'}, 't4': {'value': 9223372036854775807, 'type': 'bigint'}, 't5': {'value': 11.12345027923584, 'type': 'float'}, 't6': {'value': 22.123456789, 'type': 'double'}, 't7': {'value': 'tuzsfrom', 'type': 'binary'}, 'id': tb_name}}, 'punftb'), 
                                    ({'metric': stb_name, 'timestamp': {'value': 0, 'type': 'ns'}, 'value': {'value': 'llqzvgvw', 'type': 'binary'}, 'tags': {'t0': {'value': False, 'type': 'bool'}, 't1': {'value': 127, 'type': 'tinyint'}, 't2': {'value': 32767, 'type': 'smallint'}, 't3': {'value': 2147483647, 'type': 'int'}, 't4': {'value': 9223372036854775807, 'type': 'bigint'}, 't5': {'value': 11.12345027923584, 'type': 'float'}, 't6': {'value': 22.123456789, 'type': 'double'}, 't7': {'value': 'nttjdzgi', 'type': 'binary'}, 'id': tb_name}}, 'punftb'), 
                                    ({'metric': stb_name, 'timestamp': {'value': 0, 'type': 'ns'}, 'value': {'value': 'tclbosqc', 'type': 'binary'}, 'tags': {'t0': {'value': False, 'type': 'bool'}, 't1': {'value': 127, 'type': 'tinyint'}, 't2': {'value': 32767, 'type': 'smallint'}, 't3': {'value': 2147483647, 'type': 'int'}, 't4': {'value': 9223372036854775807, 'type': 'bigint'}, 't5': {'value': 11.12345027923584, 'type': 'float'}, 't6': {'value': 22.123456789, 'type': 'double'}, 't7': {'value': 'uatpzgpi', 'type': 'binary'}, 'id': tb_name}}, 'punftb'), 
                                    ({'metric': stb_name, 'timestamp': {'value': 0, 'type': 'ns'}, 'value': {'value': 'rlpuzodt', 'type': 'binary'}, 'tags': {'t0': {'value': True, 'type': 'bool'}, 't1': {'value': 127, 'type': 'tinyint'}, 't2': {'value': 32767, 'type': 'smallint'}, 't3': {'value': 2147483647, 'type': 'int'}, 't4': {'value': 9223372036854775807, 'type': 'bigint'}, 't5': {'value': 11.12345027923584, 'type': 'float'}, 't6': {'value': 22.123456789, 'type': 'double'}, 't7': {'value': 'cwnpdnng', 'type': 'binary'}, 'id': tb_name}}, 'punftb'), 
                                    ({'metric': stb_name, 'timestamp': {'value': 0, 'type': 'ns'}, 'value': {'value': 'rhnikvfq', 'type': 'binary'}, 'tags': {'t0': {'value': True, 'type': 'bool'}, 't1': {'value': 127, 'type': 'tinyint'}, 't2': {'value': 32767, 'type': 'smallint'}, 't3': {'value': 2147483647, 'type': 'int'}, 't4': {'value': 9223372036854775807, 'type': 'bigint'}, 't5': {'value': 11.12345027923584, 'type': 'float'}, 't6': {'value': 22.123456789, 'type': 'double'}, 't7': {'value': 'afcibyeb', 'type': 'binary'}, 'id': tb_name}}, 'punftb')]
        self.multiThreadRun(self.genMultiThreadSeq(s_stb_s_tb_d_ts_m_tag_list))
        tdSql.query(f"show tables;")
        tdSql.checkRows(1)
        tdSql.query(f"select * from {stb_name}")
        tdSql.checkRows(6)
        tdSql.query(f"select * from {stb_name} where t8 is not NULL")
        tdSql.checkRows(6)

    def sStbStbDdataDtsAtInsertMultiThreadCheckCase(self):
        """
            thread input same stb tb, different ts, add tag, mul col
        """
        tdCom.cleanTb()
        tb_name = tdCom.getLongName(7, "letters")
        input_json, stb_name = self.genFullTypeJson(tb_name=tb_name, col_value=self.genTsColValue(value="binaryTagValue", t_type="binary"))
        self.resCmp(input_json, stb_name)
        s_stb_s_tb_d_ts_a_tag_list = [({'metric': stb_name, 'timestamp': {'value': 0, 'type': 'ns'}, 'value': {'value': 'pjndapjb', 'type': 'binary'}, 'tags': {'t0': {'value': False, 'type': 'bool'}, 't1': {'value': 127, 'type': 'tinyint'}, 't2': {'value': 32767, 'type': 'smallint'}, 't3': {'value': 2147483647, 'type': 'int'}, 't4': {'value': 9223372036854775807, 'type': 'bigint'}, 't5': {'value': 11.12345027923584, 'type': 'float'}, 't6': {'value': 22.123456789, 'type': 'double'}, 't7': {'value': 'tuzsfrom', 'type': 'binary'}, 't8': {'value': 'ncharTagValue', 'type': 'nchar'}, 't11': {'value': 127, 'type': 'tinyint'}, 't10': {'value': 'ncharTagValue', 'type': 'nchar'}, 'id': tb_name}}, 'punftb'), 
                                    ({'metric': stb_name, 'timestamp': {'value': 0, 'type': 'ns'}, 'value': {'value': 'llqzvgvw', 'type': 'binary'}, 'tags': {'t0': {'value': False, 'type': 'bool'}, 't1': {'value': 127, 'type': 'tinyint'}, 't2': {'value': 32767, 'type': 'smallint'}, 't3': {'value': 2147483647, 'type': 'int'}, 't4': {'value': 9223372036854775807, 'type': 'bigint'}, 't5': {'value': 11.12345027923584, 'type': 'float'}, 't6': {'value': 22.123456789, 'type': 'double'}, 't7': {'value': 'nttjdzgi', 'type': 'binary'}, 't8': {'value': 'ncharTagValue', 'type': 'nchar'}, 't11': {'value': 127, 'type': 'tinyint'}, 't10': {'value': 'ncharTagValue', 'type': 'nchar'}, 'id': tb_name}}, 'punftb'), 
                                    ({'metric': stb_name, 'timestamp': {'value': 0, 'type': 'ns'}, 'value': {'value': 'tclbosqc', 'type': 'binary'}, 'tags': {'t0': {'value': False, 'type': 'bool'}, 't1': {'value': 127, 'type': 'tinyint'}, 't2': {'value': 32767, 'type': 'smallint'}, 't3': {'value': 2147483647, 'type': 'int'}, 't4': {'value': 9223372036854775807, 'type': 'bigint'}, 't5': {'value': 11.12345027923584, 'type': 'float'}, 't6': {'value': 22.123456789, 'type': 'double'}, 't7': {'value': 'uatpzgpi', 'type': 'binary'}, 't8': {'value': 'ncharTagValue', 'type': 'nchar'}, 't11': {'value': 127, 'type': 'tinyint'}, 't10': {'value': 'ncharTagValue', 'type': 'nchar'}, 'id': tb_name}}, 'punftb'), 
                                    ({'metric': stb_name, 'timestamp': {'value': 0, 'type': 'ns'}, 'value': {'value': 'rlpuzodt', 'type': 'binary'}, 'tags': {'t0': {'value': False, 'type': 'bool'}, 't1': {'value': 127, 'type': 'tinyint'}, 't2': {'value': 32767, 'type': 'smallint'}, 't3': {'value': 2147483647, 'type': 'int'}, 't4': {'value': 9223372036854775807, 'type': 'bigint'}, 't5': {'value': 11.12345027923584, 'type': 'float'}, 't6': {'value': 22.123456789, 'type': 'double'}, 't7': {'value': 'cwnpdnng', 'type': 'binary'}, 't8': {'value': 'ncharTagValue', 'type': 'nchar'}, 't11': {'value': 127, 'type': 'tinyint'}, 't10': {'value': 'ncharTagValue', 'type': 'nchar'}, 'id': tb_name}}, 'punftb'), 
                                    ({'metric': stb_name, 'timestamp': {'value': 0, 'type': 'ns'}, 'value': {'value': 'rhnikvfq', 'type': 'binary'}, 'tags': {'t0': {'value': True, 'type': 'bool'}, 't1': {'value': 127, 'type': 'tinyint'}, 't2': {'value': 32767, 'type': 'smallint'}, 't3': {'value': 2147483647, 'type': 'int'}, 't4': {'value': 9223372036854775807, 'type': 'bigint'}, 't5': {'value': 11.12345027923584, 'type': 'float'}, 't6': {'value': 22.123456789, 'type': 'double'}, 't7': {'value': 'afcibyeb', 'type': 'binary'}, 't8': {'value': 'ncharTagValue', 'type': 'nchar'}, 't11': {'value': 127, 'type': 'tinyint'}, 't10': {'value': 'ncharTagValue', 'type': 'nchar'}, 'id': tb_name}}, 'punftb')]
        self.multiThreadRun(self.genMultiThreadSeq(s_stb_s_tb_d_ts_a_tag_list))
        tdSql.query(f"show tables;")
        tdSql.checkRows(1)
        tdSql.query(f"select * from {stb_name}")
        tdSql.checkRows(6)
        for t in ["t10", "t11"]:
            tdSql.query(f"select * from {stb_name} where {t} is not NULL;")
            tdSql.checkRows(0)

    def sStbDtbDdataDtsInsertMultiThreadCheckCase(self):
        """
            thread input same stb, different tb, data, ts
        """
        tdCom.cleanTb()
        input_json, stb_name = self.genFullTypeJson(col_value=self.genTsColValue(value="binaryTagValue", t_type="binary"))
        self.resCmp(input_json, stb_name)
        s_stb_d_tb_d_ts_list = self.genSqlList(stb_name=stb_name)[10]
        self.multiThreadRun(self.genMultiThreadSeq(s_stb_d_tb_d_ts_list))
        tdSql.query(f"show tables;")
        tdSql.checkRows(6)

    def sStbDtbDdataDtsMtInsertMultiThreadCheckCase(self):
        """
            thread input same stb, different tb, data, ts, add col, mul tag
        """
        tdCom.cleanTb()
        input_json, stb_name = self.genFullTypeJson(col_value=self.genTsColValue(value="binaryTagValue", t_type="binary"))
        self.resCmp(input_json, stb_name)
        s_stb_d_tb_d_ts_m_tag_list = [({'metric': stb_name, 'timestamp': {'value': 0, 'type': 'ns'}, 'value': {'value': 'pjndapjb', 'type': 'binary'}, 'tags': {'t0': {'value': False, 'type': 'bool'}, 't1': {'value': 127, 'type': 'tinyint'}, 't2': {'value': 32767, 'type': 'smallint'}, 't3': {'value': 2147483647, 'type': 'int'}, 't4': {'value': 9223372036854775807, 'type': 'bigint'}, 't5': {'value': 11.12345027923584, 'type': 'float'}, 't6': {'value': 22.123456789, 'type': 'double'}}}, 'punftb'), 
                                    ({'metric': stb_name, 'timestamp': {'value': 0, 'type': 'ns'}, 'value': {'value': 'llqzvgvw', 'type': 'binary'}, 'tags': {'t0': {'value': True, 'type': 'bool'}, 't1': {'value': 127, 'type': 'tinyint'}, 't2': {'value': 32767, 'type': 'smallint'}, 't3': {'value': 2147483647, 'type': 'int'}, 't4': {'value': 9223372036854775807, 'type': 'bigint'}, 't5': {'value': 11.12345027923584, 'type': 'float'}, 't6': {'value': 22.123456789, 'type': 'double'}}}, 'punftb'), 
                                    ({'metric': stb_name, 'timestamp': {'value': 0, 'type': 'ns'}, 'value': {'value': 'tclbosqc', 'type': 'binary'}, 'tags': {'t0': {'value': False, 'type': 'bool'}, 't1': {'value': 127, 'type': 'tinyint'}, 't2': {'value': 32767, 'type': 'smallint'}, 't3': {'value': 2147483647, 'type': 'int'}, 't4': {'value': 9223372036854775807, 'type': 'bigint'}, 't5': {'value': 11.12345027923584, 'type': 'float'}, 't6': {'value': 22.123456789, 'type': 'double'}}}, 'punftb'), 
                                    ({'metric': stb_name, 'timestamp': {'value': 0, 'type': 'ns'}, 'value': {'value': 'rlpuzodt', 'type': 'binary'}, 'tags': {'t0': {'value': False, 'type': 'bool'}, 't1': {'value': 127, 'type': 'tinyint'}, 't2': {'value': 32767, 'type': 'smallint'}, 't3': {'value': 2147483647, 'type': 'int'}, 't4': {'value': 9223372036854775807, 'type': 'bigint'}, 't5': {'value': 11.12345027923584, 'type': 'float'}, 't6': {'value': 22.123456789, 'type': 'double'}}}, 'punftb'), 
                                    ({'metric': stb_name, 'timestamp': {'value': 0, 'type': 'ns'}, 'value': {'value': 'rhnikvfq', 'type': 'binary'}, 'tags': {'t0': {'value': False, 'type': 'bool'}, 't1': {'value': 127, 'type': 'tinyint'}, 't2': {'value': 32767, 'type': 'smallint'}, 't3': {'value': 2147483647, 'type': 'int'}, 't4': {'value': 9223372036854775807, 'type': 'bigint'}, 't5': {'value': 11.12345027923584, 'type': 'float'}, 't6': {'value': 22.123456789, 'type': 'double'}}}, 'punftb')]
        self.multiThreadRun(self.genMultiThreadSeq(s_stb_d_tb_d_ts_m_tag_list))
        tdSql.query(f"show tables;")
        tdSql.checkRows(3)

    def test(self):
        # input_sql1 = "stb2_5 1626006833610ms 3f64 host=\"host0\",host2=L\"host2\""
        # input_sql2 = "rfasta,id=\"rfasta_1\",t0=true,t1=127i8,t2=32767i16,t3=2147483647i32,t4=9223372036854775807i64,t5=11.12345f32,t6=22.123456789f64 c0=True,c1=127i8,c2=32767i16,c3=2147483647i32,c4=9223372036854775807i64,c5=11.12345f32,c6=22.123456789f64 1626006933640000000ns"
        try:
            input_json = f'test_nchar 0 L"涛思数据" t0=f,t1=L"涛思数据",t2=32767i16,t3=2147483647i32,t4=9223372036854775807i64,t5=11.12345f32,t6=22.123456789f64'
            self._conn.insert_lines([json.dumps(input_json)], 2)
            # input_json, stb_name = self.genFullTypeJson()
            # self.resCmp(input_json, stb_name)        
        except LinesError as err:
            print(err.errno)
        # self._conn.insert_json_payload([input_sql2])
        # input_sql3 = f'abcd,id="cc¥Ec",t0=True,t1=127i8,t2=32767i16,t3=2147483647i32,t4=9223372036854775807i64,t5=11.12345f32,t6=22.123456789f64,t7="ndsfdrum",t8=L"ncharTagValue" c0=f,c1=127i8,c2=32767i16,c3=2147483647i32,c4=9223372036854775807i64,c5=11.12345f32,c6=22.123456789f64,c7="igwoehkm",c8=L"ncharColValue",c9=7u64 0'
        # print(input_sql3)
        # input_sql4 = 'hmemeb,id="kilrcrldgf",t0=F,t1=127i8,t2=32767i16,t3=2147483647i32,t4=9223372036854775807i64,t5=11.12345f32,t6=22.123456789f64,t7="fysodjql",t8=L"ncharTagValue" c0=True,c1=127i8,c2=32767i16,c3=2147483647i32,c4=9223372036854775807i64,c5=11.12345f32,c6=22.123456789f64,c7="waszbfvc",c8=L"ncharColValue",c9=7u64 0'
        # code = self._conn.insert_json_payload([input_sql3])
        # print(code)
        # self._conn.insert_json_payload([input_sql4])

    def runAll(self):
        for value_type in ["obj", "default"]:
            self.initCheckCase(value_type)
            self.boolTypeCheckCase()
            self.symbolsCheckCase(value_type)
            self.tsCheckCase(value_type)
            self.idSeqCheckCase(value_type)
            self.idUpperCheckCase(value_type)
            self.noIdCheckCase(value_type)
            self.maxColTagCheckCase(value_type)
            self.idIllegalNameCheckCase(value_type)
            self.idStartWithNumCheckCase(value_type)
            self.nowTsCheckCase(value_type)
            self.dateFormatTsCheckCase(value_type)
            self.illegalTsCheckCase(value_type)
            self.tagValueLengthCheckCase(value_type)
            self.colValueLengthCheckCase(value_type)
            self.tagColIllegalValueCheckCase(value_type)
        # #! bug
        # self.duplicateIdTagColInsertCheckCase()
        # self.noIdStbExistCheckCase()
        # self.duplicateInsertExistCheckCase()
        # self.tagColBinaryNcharLengthCheckCase()
        # #! bug
        # # self.lengthIcreaseCrashCheckCase()
        # self.tagColAddDupIDCheckCase()
        # self.tagColAddCheckCase()
        # self.tagMd5Check()
        # self.tagColBinaryMaxLengthCheckCase()
        # self.tagColNcharMaxLengthCheckCase()
        # self.batchInsertCheckCase()
        # self.multiInsertCheckCase(10)
        # self.batchErrorInsertCheckCase()
        # self.multiColsInsertCheckCase()
        # self.blankColInsertCheckCase()
        # self.blankTagInsertCheckCase()
        # self.chineseCheckCase()
        # self.multiFieldCheckCase()
        # self.spellCheckCase()
        # self.pointTransCheckCase()
        # # # MultiThreads
        # self.stbInsertMultiThreadCheckCase()
        # self.sStbStbDdataInsertMultiThreadCheckCase()
        # self.sStbStbDdataAtInsertMultiThreadCheckCase()
        # self.sStbStbDdataMtInsertMultiThreadCheckCase()
        # self.sStbDtbDdataInsertMultiThreadCheckCase()
        # self.sStbDtbDdataMtInsertMultiThreadCheckCase()
        # self.sStbDtbDdataAtInsertMultiThreadCheckCase()
        # self.sStbStbDdataDtsInsertMultiThreadCheckCase()
        # #! run fail
        # self.sStbStbDdataDtsMtInsertMultiThreadCheckCase()
        # self.sStbStbDdataDtsAtInsertMultiThreadCheckCase()
        # self.sStbDtbDdataDtsInsertMultiThreadCheckCase()
        # self.sStbDtbDdataDtsMtInsertMultiThreadCheckCase()

    def run(self):
        print("running {}".format(__file__))
        self.createDb()
        try:
            self.runAll()
        except Exception as err:
            print(''.join(traceback.format_exception(None, err, err.__traceback__)))
            raise err

    def stop(self):
        tdSql.close()
        tdLog.success("%s successfully executed" % __file__)

tdCases.addWindows(__file__, TDTestCase())
tdCases.addLinux(__file__, TDTestCase())
