[toc]

## 前言

### 准备mysql环境

详细可参考：[mysql环境准备](https://da1234cao.blog.csdn.net/article/details/105478479)。

下面是简单步骤。环境是ubuntu20，mysql版本是8.0.30

```shell
sudo apt-get install mysql-server # mysql数据库服务端
sudo apt install mysql-workbench-community # mysql数据库图形化客户端
sudo apt install libmysqlcppconn-dev # mysql C++接口的头文件和库

# 数据库root用户默认设置是无法远程登录。系统root用户下，可以免密登录数据库
# 使用系统root免密登录，数据库root用户；在数据库root用户下，创建一个可以远程访问的新用户
su -
mysql -u root
CREATE USER  'dacao'@'%'  IDENTIFIED BY 'qwert';

# 创建数据库
create database message_board

# 将创建的数据库的所有权限给新创建的用户
GRANT ALL privileges  ON message_board.*  TO 'dacao'@'%';

# 这时候，我们可以退出root用户，使用mysql-workbench连接我们上面创建的数据库
# 连接成功后，我们创建一个表
show databases;

USE message_board;

CREATE TABLE message(
    username char(50) NULL,
    message char(50) NULL
)ENGINE=InnoDB;

# INSERT INTO user(username, passwd) VALUES('zhangsan', 'hello world');
# select * from user;
```

---
### 目标-寻找一个C++的mysql数据库连接池

mysql提供了不同语言的连接接口，可见：[Connectors and APIs](https://dev.mysql.com/doc/index-connectors.html)

第一种是[MySQL 8.0 C API Developer Guide](https://dev.mysql.com/doc/c-api/8.0/en/)。网上有不同的封装，大同小异，可参考参考：[C++ mysql连接池 附带讲解与详细注释源码](https://blog.csdn.net/q2453303961/article/details/125522931)、[qinguoyi/TinyWebServer-sql_connection_pool.cpp](https://github.com/qinguoyi/TinyWebServer/blob/master/CGImysql/sql_connection_pool.cpp)。（不明白，既然官方已经提供了mysq的c++ api,为啥还要再封装mysql的C接口。）

第二种是[MySQL Connector/C++ 8.0 Developer Guide](https://dev.mysql.com/doc/connector-cpp/8.0/en/)。第三种是[Connector/C/C++ X DevAPI Reference](https://dev.mysql.com/doc/dev/connector-cpp/8.0/)。由于我之前整理过[C++连接MySQL数据库](https://da1234cao.blog.csdn.net/article/details/122780034),这里不再赘述。

其中，X DevAPI提供了mysql的数据库连接池的api，可参考：[2.2.3 Connecting to a Single MySQL Server Using Connection Pooling](https://dev.mysql.com/doc/x-devapi-userguide/en/connecting-connection-pool.html)

---
## X DevAPI - 数据库连接池

没有写多线程测试，只是写了一个简单的demo跑了下。代码中涉及到表的增删改查的api，可参考[3.1 CRUD Operations Overview](https://dev.mysql.com/doc/x-devapi-userguide/en/crud-operations-overview.html)

```cpp
// compile:  g++  test-x-plugin.cpp -o test-x-plugin -I /usr/include/mysql-cppconn-8/  -lmysqlcppconn8

#include <mysql-cppconn-8/mysqlx/xdevapi.h>
#include <iostream>
#include <list>

/**
 * 数据库连接,并给表插入一列
 */
void unit_test_1(void) {
  mysqlx::Session sess("localhost", 33060, "dacao", "qwert");
  mysqlx::Schema db= sess.getSchema("message_board");
  mysqlx::Table tb = db.getTable("message");

  tb.insert("username", "message")
                               .values("lisi","hello world")
                               .execute();
}

/**
 * 数据库连接池
 */
void unit_test_2(void) {
  using namespace mysqlx;
  // Client cli("user:password@host_name/db_name", ClientOption::POOL_MAX_SIZE, 7);
  Client cli("dacao:qwert@127.0.0.1", ClientOption::POOL_MAX_SIZE, 7);
  Session sess = cli.getSession();

  mysqlx::Schema db= sess.getSchema("message_board");
  mysqlx::Table tb = db.getTable("message");
  mysqlx::RowResult result = tb.select("*").execute();
  std::list<mysqlx::Row> rows = result.fetchAll();
  std::cout<<"name "<<"password "<<std::endl;
  for(auto row : rows) {
    std::cout<<row[0]<<" "<<row[1]<<std::endl;
  }

  cli.close();  // close all Sessions
}


int main(int argc, char** argv)
{
  unit_test_1();
  unit_test_2();
}
```

上面代码需要提下的是，连接数据库的参考。第一种连接是`mysqlx::Session sess("localhost", 33060, "dacao", "qwert");`。第二种连接是`Client cli("dacao:qwert@127.0.0.1/message_board", ClientOption::POOL_MAX_SIZE, 7);`。这两种构造过程是类似的，可以看下`xdevapi.h`源文件中的注释。

```cpp
Session from_uri("mysqlx://user:pwd@host:port/db?ssl-mode=disabled");
Session from_options("host", port, "user", "pwd", "db");
Session from_option_list(
    SessionOption::USER, "user",
    SessionOption::PWD,  "pwd",
    SessionOption::HOST, "host",
    SessionOption::PORT, port,
    SessionOption::DB,   "db",
    SessionOption::SSL_MODE, SSLMode::DISABLED
    );

Client from_uri("mysqlx://user:pwd\@host:port/db?ssl-mode=disabled");
Client from_options("host", port, "user", "pwd", "db");
Client from_option_list(
    SessionOption::USER, "user",
    SessionOption::PWD,  "pwd",
    SessionOption::HOST, "host",
    SessionOption::PORT, port,
    SessionOption::DB,   "db",
    SessionOption::SSL_MODE, SSLMode::DISABLED
    ClientOption::POOLING, true,
    ClientOption::POOL_MAX_SIZE, 10,
    ClientOption::POOL_QUEUE_TIMEOUT, 1000,
    ClientOption::POOL_MAX_IDLE_TIME, 500,
);
```