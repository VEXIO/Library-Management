//
// Created by Haotian on 17/4/23.
//

#include "MyDatabase.h"
#include <stdexcept>

MyDatabase::MyDatabase(const char *Host, const char *User, const char *Pass, const char *Database) {
    mysql_init(&mysql);

    conn = mysql_real_connect(&mysql, Host, User, Pass, Database, 0, 0, 0);

    if (!conn) {
        throw std::runtime_error("Unable to connect to DB.");
    } else {
        status = true;
    }
}

MyDatabase::~MyDatabase() {
    free();
    mysql_close(conn);
    status = false;
}

bool MyDatabase::isConnected() const {
    return status && conn;
}

void MyDatabase::assertConnect() const {
    if (!isConnected()) {
        throw std::runtime_error("DB disconnected.");
    }
}

long MyDatabase::raw(const char *query) {
    int queryStatus = mysql_query(conn, query);
    if (!queryStatus) {
        return static_cast<long>(mysql_affected_rows(conn));
    } else {
        throw std::runtime_error(showError());
    }
}

std::string MyDatabase::showError() const {
    std::string err = mysql_error(conn);
    while (err[0] == ' ') { err.erase(0, 1); }
    return std::string(err);
}

void MyDatabase::escape(std::string &var) const {
    char *newVar = new char[var.length() * 2];
    mysql_real_escape_string(conn, newVar, var.c_str(), var.length());
    var = newVar;
    delete[] newVar;
}

unsigned int MyDatabase::select(const char *query) {
    assertConnect();
    raw(query);
    free();
    last_result = mysql_store_result(conn);
    unsigned int num_rows = mysql_num_rows(last_result);
    return num_rows;
}

unsigned int MyDatabase::select(std::string &query) {
    return select(query.c_str());
}

MyRow MyDatabase::row() {
    assertConnect();
    if (last_result == nullptr) {
        throw std::runtime_error("Result is empty");
    }
    unsigned int num_fields = mysql_num_fields(last_result);
    return MyRow(num_fields, mysql_fetch_row(last_result));
}

std::vector<MyRow> MyDatabase::result() {
    std::vector<MyRow> resultVec;
    MYSQL_ROW row;
    unsigned int num_fields = mysql_num_fields(last_result);
    while (row = mysql_fetch_row(last_result)) {
        // combine rows to result
        resultVec.push_back(MyRow(num_fields, row));
    }
    return resultVec;
}

bool MyDatabase::free() {
    // free if last reseult exists
    if (last_result != nullptr) {
        mysql_free_result(last_result);
        last_result = nullptr;
        return true;
    } else {
        return false;
    }
}

long MyDatabase::update(const char *query) {
    assertConnect();
    return raw(query);
}

long MyDatabase::update(std::string &query) {
    return update(query.c_str());
}

void MyDatabase::fixDefault(std::string &var) const {
    if (var == "") {
        var = "default";
    } else {
        var = "'" + var + "'";
    }
}

bool MyDatabase::isInt(std::string &var) const {
    long long nvar;
    std::string svar;
    try {
        nvar = std::stoll(var);
    } catch (std::exception &e) {
        return false;
    }
    svar = std::to_string(nvar);
    // only when it is completely int (or something like 11d will be allowed)
    return svar == var;
}
