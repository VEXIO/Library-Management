//
// Created by Haotian on 17/4/23.
//

#ifndef LIBRARY_MANAGEMENT_MYDATABASE_H
#define LIBRARY_MANAGEMENT_MYDATABASE_H

#include <mysql/mysql.h>
#include <string>
#include <vector>
#include <iostream>

struct MyRow {
    MyRow() {}

    MyRow(unsigned int fields, MYSQL_ROW rowData) : num_fields(fields), row(rowData) {}

    unsigned int num_fields;
    MYSQL_ROW row;
};

class MyDatabase {
public:
    // constructor
    MyDatabase(const char *Host, const char *User, const char *Pass, const char *Database);

    // destructor
    ~MyDatabase();

    // pass std::string query to const char *query using std::string::c_str()
    unsigned int select(std::string &query);

    // select query and store result to last_result for later fetch and return result row count
    unsigned int select(const char *query);

    // pass std::string query to const char *query using std::string::c_str()
    long update(const char *query);

    // update query and return affected row count
    long update(std::string &query);

    // return row if last_result exists
    MyRow row();

    // return table of rows if last_result exists
    std::vector<MyRow> result();

    // free last_result if exists
    bool free();

    // escape var to avoid SQL injection
    void escape(std::string &var) const;

    // set value to default if it is null, or add ''
    void fixDefault(std::string &var) const;

    // judge if it is int
    bool isInt(std::string &var) const;

private:
    MYSQL mysql;
    MYSQL *conn;
    MYSQL_RES *last_result = nullptr;
    bool status = false;

    bool isConnected() const;

    void assertConnect() const;

    long raw(const char *query);

    std::string showError() const;
};


#endif //LIBRARY_MANAGEMENT_MYDATABASE_H
