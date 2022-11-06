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