//
// Created by Haotian on 17/4/23.
//

#ifndef LIBRARY_MANAGEMENT_LIBRARY_H
#define LIBRARY_MANAGEMENT_LIBRARY_H

#include "MyDatabase.h"
#include <vector>
#include <string>
#include <map>
#include <iostream>

struct UserInfo {
    UserInfo() : loggedIn(false) {}

    bool loggedIn;
    std::string stuid;
    int authLevel;
};

class Library {
public:
    // constructor
    Library(MyDatabase *db) : db(db) {}

    // run the program
    void run();

private:
    MyDatabase *db;
    UserInfo user;
    bool halt = false;

    // check whether user is logged in
    bool isLoggedIn() const { return user.loggedIn; }

    // authorize user password
    void authorize();

    // print user panel
    void panel();

    // manage readers (list)
    void manageReader();

    // manage books (list)
    void manageBook();

    // list books available
    void listBook();

    // add one book with several instances
    void addBook();

    // remove a book and delete all instances
    void deleteBook();

    // circulate book (list)
    void circulateBook();

    // borrow one book
    void borrowBook();

    // return a book
    void returnBook();

    // list all readers
    void listReader();

    // add a reader
    void addReader();

    // remove a reader
    void deleteReader();

    // user log out
    void doLogOut();
};


#endif //LIBRARY_MANAGEMENT_LIBRARY_H
