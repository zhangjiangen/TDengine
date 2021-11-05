package com.taosdata.jdbc.cases;

import org.junit.Test;

import java.sql.*;

public class DelimiterInSqlTest {

    //    private static final String host = "127.0.0.1";
    private static final String host = "master";

    @Test
    public void test() {
        try {
            Connection conn = DriverManager.getConnection("jdbc:TAOS://" + host + ":6030/?user=root&password=taosdata");
            Statement stmt = conn.createStatement();

            stmt.execute("drop database if exists test_delimiter");
            stmt.execute("create database if not exists test_delimiter");
            stmt.execute("use test_delimiter");
            stmt.execute("create table api_gateway_log(ts TIMESTAMP, insert_time TIMESTAMP, name NCHAR(500), http_type NCHAR(200), url NCHAR(1000), costs INT, header NCHAR(1000), params NCHAR(1000), body NCHAR(1000), rsp_status INT, resbody NCHAR(1000), req_type TINYINT, http_id INT, content_type NCHAR(500), file_size BIGINT) tags(vin NCHAR(200), user_id NCHAR(200))");
            stmt.execute("insert into api_gateway_log_null using api_gateway_log(user_id) tags(null) values('2021-11-02 09:27:19.848', now, '车主年度报表数据', 'GET', 'https://devapigateway.leapmotor.com/carownerservice/v1/api/annualReport/data?carownerUrl=&t=data%00&year=2019', 39, 'Host:devapigateway.leapmotor.com= X-Request-ID:5f7d23844c44dde9140be191a3b29e1c= X-Real-IP:10.192.126.229= X-Forwarded-For:10.192.126.229= X-Forwarded-Host:devapigateway.leapmotor.com= X-Forwarded-Port:443= X-Forwarded-Proto:https= X-Original-URI:/carownerservice/v1/api/annualReport/data?carownerUrl=&t=data%00&year=2019= X-Scheme:https= Accept:*/*= User-Agent:node-fetch/1.0 #+https://github.com/bitinn/node-fetch#= Accept-Encoding:gzip,deflate ', 'carownerUrl:= t:data= year:2019 ', 'null', 400, '{\"timestamp\":\"2021-11-02T01:27:04.041+0000\",\"status\":400,\"error\":\"Bad Request\",\"message\":\"Required String parameter #vin# is not present\",\"path\":\"/carownerservice/v1/api/annualReport/data\"}', 2, 314, 'null', null)");

            stmt.close();
            conn.close();

        } catch (SQLException e) {
            e.printStackTrace();
        }
    }



}