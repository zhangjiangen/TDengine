package com.taosdata.jdbc.cases;

import com.taosdata.jdbc.TSDBConnection;
import com.taosdata.jdbc.TSDBSubscribe;
import org.junit.Test;

import java.sql.*;
import java.util.List;
import java.util.UUID;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;
import java.util.stream.IntStream;

public class SubscribeSameTableDifferentDbname {

    @Test
    public void test() throws SQLException, InterruptedException {
        // given
        String host = "master";
        Connection connection1 = DriverManager.getConnection("jdbc:TAOS://" + host + ":6030/test?user=root&password=taosdata");
        Connection connection2 = DriverManager.getConnection("jdbc:TAOS://" + host + ":6030/test2?user=root&password=taosdata");

        Connection[] connections = new Connection[2];
        connections[0] = connection1;
        connections[1] = connection2;


        List<Thread> threads = IntStream.range(1, 3).mapToObj(i -> new Thread(() -> {
            SubscribeHandler handler = new SubscribeHandler(connections[i - 1]);
            try {
                while (true) {
                    final ResultSet fetch = handler.fetch();
                    if (i == 1)
                        processResult(fetch, data -> System.out.println(data));
                    else
                        processResult(fetch, data -> System.err.println(data));
                    TimeUnit.MICROSECONDS.sleep(10);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        })).collect(Collectors.toList());

        for (Thread thread : threads) {
            thread.start();
        }

        for (Thread thread : threads) {
            thread.join();
        }

        connection1.close();
        connection2.close();
    }

    @FunctionalInterface
    private interface QueryCallback {
        void call(Object data);
    }

    private static int processResult(ResultSet resultSet, QueryCallback callback) throws Exception {
        int count = 0;
        while (true) {
            try {
                if (!resultSet.next()) {
                    break;
                }
            } catch (Exception e) {
                break;
            }
            ResultSetMetaData metaData = resultSet.getMetaData();
            try {
                int columnCount = metaData.getColumnCount();
                Object data = new Object();
                for (int i = 1; i <= columnCount; i++) {
                    System.out.print(metaData.getColumnLabel(i) + ": " + resultSet.getString(i) + "\t");
                }
                if (null != callback) {
                    callback.call(data);
                }

                count++;
            } catch (Exception e) {
                break;
            }
        }
        return count;
    }

    public class SubscribeHandler {

        private TSDBSubscribe subscribe;
        private final Connection connection;

        public SubscribeHandler(Connection connection) {
            this.connection = connection;
        }

        public ResultSet fetch() throws Exception {
            if (null == subscribe) {
                String sql = "select * from meters";
                // 连接,使用的hcp连接池
                subscribe = connection.unwrap(TSDBConnection.class).subscribe(UUID.randomUUID().toString(), sql, false);
            }
            return subscribe.consume();
        }
    }
}
