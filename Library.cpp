//
// Created by Haotian on 17/4/23.
//

#include "Library.h"
#include <iomanip>

const int LibraryDefaultWidth = 30;

inline void hline(int width = LibraryDefaultWidth) {
    std::cout << "+";
    for (int i = 2; i < width; i++) { std::cout << "-"; }
    std::cout << "+" << std::endl;
}

// draw aligned row
inline void srow(std::vector<std::string> &row, std::vector<int> &width) {
    std::cout << "|";
    int wid, blank1, blank2;
    for (int i = 0; i < row.size(); i++) {
        wid = width[i];
        // calculate first blank
        blank1 = static_cast<int>(width[i] + row[i].length()) / 2;
        // fit rest width with blanks
        blank2 = wid - blank1;
        // print row content with blanks
        std::cout << std::setw(blank1) << row[i]
                  << std::setw(blank2) << "" << "|";
    }
    std::cout << std::endl;
}

inline void sline(std::string str, int width = LibraryDefaultWidth, bool align = false) {
    if (align) {
        std::vector<std::string> row = {str};
        std::vector<int> ww = {width - 2};
        // need align, generate fake row with 1 column
        srow(row, ww);
    } else {
        std::cout << "| " << str;
        for (long i = 4 + str.length(); i < width; i++) { std::cout << " "; }
        std::cout << " |" << std::endl;
    }
}

// draw head block
inline void hblock(std::string str) { hline(), sline(str, LibraryDefaultWidth, true), hline(); }

// draw panel body rows (un-aligned)
inline void sblock(std::string str) { sline(str), hline(); }

inline void mkPanel(const char *title, std::vector<std::string> &ops) {
    hblock(title);
    int size = ops.size();
    for (int i = 0; i < size - 1; i++) {
        sline(std::to_string(i + 1) + ". " + ops[i]);
    }
    sblock(std::to_string(size) + ". " + ops[size - 1]);
}

void Library::run() {
    hblock("Library Management System");
    std::cout << std::endl;
    while (!halt) {
        authorize();
        panel();
    }
}

void Library::authorize() {
    std::string stuid, password;
    std::string sql_query;
    MyRow user;
    int num_rows;
    while (!isLoggedIn()) {
        std::cout << "Username: ";
        std::cin >> stuid;
        std::cout << "Password: ";
        std::cin >> password;
        // escape input
        db->escape(stuid);
        db->escape(password);
        // query to check row
        sql_query =
                "SELECT stuid, auth_level FROM user WHERE stuid='" + stuid + "' AND password=MD5('" + password + "')";
        num_rows = db->select(sql_query);
        // when row count positive, fetch row to user
        if (num_rows > 0 && stuid == (user = db->row()).row[0]) {
            // store user info
            this->user.loggedIn = true;
            this->user.stuid = user.row[0];
            this->user.authLevel = std::stoi(user.row[1]);
            std::cout << "Logged In!" << std::endl;
        } else {
            db->free();
            std::cout << "Authorization failed, please try again." << std::endl;
        }
    }
}

void Library::panel() {
    int op;
    // loop if program not halt and user did not choose to log out
    while (!halt && isLoggedIn()) {
        // for user without super level auth, only allow circulate book
        if (user.authLevel != 9) {
            std::vector<std::string> ops = {
                    "Circulate Book",
                    "Log out",
                    "Exit"
            };
            mkPanel("Library Control Panel", ops);
            std::cout << "Input option No: ";
            std::cin >> op;
            switch (op) {
                case 1:
                    circulateBook();
                    break;
                case 2:
                    doLogOut();
                    break;
                case 3:
                    halt = true;
                    break;
                default:
                    std::cout << "Invalid opcode." << std::endl;
                    if (std::cin.fail()) { std::cin.clear(); }
                    break;
            }
        } else {
            // for admins, they have super authority.
            std::vector<std::string> ops = {
                    "Manage Readers",
                    "Manage Books",
                    "Circulate Book",
                    "Log out",
                    "Exit"
            };
            mkPanel("Library Control Panel", ops);
            std::cout << "Input option No: ";
            std::cin >> op;
            switch (op) {
                case 1:
                    manageReader();
                    break;
                case 2:
                    manageBook();
                    break;
                case 3:
                    circulateBook();
                    break;
                case 4:
                    doLogOut();
                    break;
                case 5:
                    halt = true;
                    break;
                default:
                    std::cout << "Invalid opcode." << std::endl;
                    if (std::cin.fail()) { std::cin.clear(); }
                    break;
            }
        }
    }
}

void Library::manageReader() {
    int op;
    bool exit = false;
    while (!exit) {
        std::vector<std::string> dops = {
                "List Readers",
                "Add Reader",
                "Delete Reader",
                "Return"
        };
        mkPanel("Manage Readers", dops);
        std::cout << "Input option No: ";
        std::cin >> op;
        switch (op) {
            case 1:
                listReader();
                break;
            case 2:
                addReader();
                break;
            case 3:
                deleteReader();
                break;
            case 4:
                exit = true;
                break;
            default:
                std::cout << "Invalid opcode." << std::endl;
                // clear cin if it is a illness input
                if (std::cin.fail()) { std::cin.clear(); }
                break;
        }
    }
}

void Library::listReader() {
    std::string sql_query = "SELECT stuid, name, allow_borrow, borrowed FROM user";
    int num_rows = db->select(sql_query);
    if (num_rows == 0) {
        std::cout << "No user exists" << std::endl;
    } else {
        auto result = db->result();
        // specify row width
        std::vector<int> width = {12, 10, 7, 10};
        std::vector<std::string> vrow = {"ID", "Name", "Allow", "Borrowed"};
        db->free();
        hline(44);
        srow(vrow, width);
        // generate each row and use srow to print aligned rows
        for (auto row = result.begin(); row < result.end(); row++) {
            for (int i = 0; i < row->num_fields; i++) {
                vrow[i] = row->row[i];
            }
            srow(vrow, width);
        }
        hline(44);
    }
}

void Library::addReader() {
    std::string stuid, password, name, allow_borrow, auth;
    std::string sql_query;
    std::cout << "Please input reader ID: " << std::endl;
    std::cin >> stuid;
    db->escape(stuid);
    std::cout << "Please input reader's name: " << std::endl;
    std::cin >> name;
    db->escape(name);
    std::cout << "Please input reader's password: " << std::endl;
    std::cin >> password;
    db->escape(password);
    // get char to ignore '\n'
    std::getchar();
    std::cout << "How many books are the reader allowed to borrow? (leave blank for default)" << std::endl;
    std::getline(std::cin, allow_borrow);
    db->escape(allow_borrow);
    // if is empty, set to default, else set to 'val'
    db->fixDefault(allow_borrow);
    std::cout << "What's reader's level? (leave blank for default)" << std::endl;
    std::getline(std::cin, auth);
    db->escape(auth);
    // same as above
    db->fixDefault(auth);

    // query insertion
    sql_query = "INSERT INTO user (stuid, password, name, allow_borrow, auth_level) VALUES ('"
                + stuid + "', MD5('" + password + "'), '" + name + "', " + allow_borrow + ", " + auth + ")";
    long affected;
    try {
        affected = db->update(sql_query);
    } catch (std::exception &e) {
        std::cout << "Input error, please try again.";
        affected = -1;
    }
    // return messages
    if (affected == 1) {
        std::cout << "Reader is added successfully!";
    } else if (affected != -1) {
        std::cout << "Insertion failed, please try again.";
    }
    std::cout << std::endl;
}

void Library::deleteReader() {
    std::string readerName;
    std::cout << "Please input reader ID: ";
    std::cin >> readerName;
    db->escape(readerName);
    std::string sql_query = "DELETE FROM user WHERE stuid=" + readerName;
    bool error_occured = false;
    long affected;
    try {
        affected = db->update(sql_query);
    } catch (std::exception &e) {
        error_occured = true;
        std::string what = e.what();
        // if exception caught with specified trigger check, print it to user
        if (what[0] == '!') {
            what.erase(0, 1); // erase the first exclamation mark
            std::cout << what << std::endl;
        } else {
            // or print generalized error.
            std::cout << "An error occured." << std::endl;
        }
    }
    if (!error_occured) {
        if (affected) {
            // success
            std::cout << "Successfully deleted the reader." << std::endl;
        } else {
            // if no affected, return empty UID
            std::cout << "Reader ID does not exist" << std::endl;
        }
    }
}

void Library::manageBook() {
    int op;
    bool exit = false;
    while (!exit) {
        std::vector<std::string> dops = {
                "List Books",
                "Add Book",
                "Delete Book",
                "Return"
        };
        mkPanel("Manage Books", dops);
        std::cout << "Input option No: ";
        std::cin >> op;
        switch (op) {
            case 1:
                listBook();
                break;
            case 2:
                addBook();
                break;
            case 3:
                deleteBook();
                break;
            case 4:
                exit = true;
                break;
            default:
                std::cout << "Invalid opcode." << std::endl;
                // clear cin if it is a illness input
                if (std::cin.fail()) { std::cin.clear(); }
                break;
        }
    }
}

void Library::listBook() {
    std::string sql_query = "SELECT id, category, cate_index, status FROM book_item";
    int num_rows = db->select(sql_query);
    if (num_rows == 0) {
        std::cout << "No book exists" << std::endl;
    } else {
        auto result = db->result();
        std::vector<int> width = {6, 10, 7, 10};
        std::vector<std::string> vrow = {"ID", "Category", "Index", "Status"};
        db->free();
        hline(38);
        srow(vrow, width);
        for (auto row = result.begin(); row < result.end(); row++) {
            for (int i = 0; i < row->num_fields; i++) {
                vrow[i] = row->row[i];
            }
            // change 0s and 1s to user-friendly expressions
            vrow[row->num_fields - 1] = (vrow[row->num_fields - 1] == "0" ? "Borrowed" : "On Shelf");
            srow(vrow, width);
        }
        hline(38);
    }
}

void Library::addBook() {
    std::string name, category, cate_index, author, publisher, year, inventory;
    std::string sql_query;
    int inv;
    std::cout << "Please input book name: " << std::endl;
    getchar();
    getline(std::cin, name);
    db->escape(name);
    std::cout << "Please input category code (like I287.1) " << std::endl;
    std::cin >> category;
    db->escape(category);
    std::cout << "Please input category index " << std::endl;
    std::cin >> cate_index;
    db->escape(cate_index);
    std::cout << "Please input the author of the book " << std::endl;
    // ignore '\n'
    getchar();
    getline(std::cin, author);
    db->escape(author);
    std::cout << "Please input the name of publisher " << std::endl;
    // ignore '\n'
    getchar();
    getline(std::cin, publisher);
    db->escape(publisher);
    std::cout << "Please input the year it is published " << std::endl;
    std::cin >> year;
    db->escape(year);

    inventory = "default";
    // loop when inventory is int
    while (!db->isInt(inventory)) {
        std::cout << "Please input the inventory of the book " << std::endl;
        std::cin >> inventory;
        db->escape(inventory);
    }

    // query insertion
    sql_query =
            "INSERT INTO book (name, category, cate_index, author, publisher, year, inventory, available) VALUES ('" +
            name +
            "','" + category + "','" + cate_index + "','" + author + "','" + publisher + "','" + year + "','" +
            inventory + "', '" + inventory + "')";

    long affected;
    try {
        affected = db->update(sql_query);
    } catch (std::exception &e) {
        std::cout << "Input error, please try again.";
        affected = -1;
    }
    if (affected == 1) {
        sql_query = "INSERT INTO book_item (category, cate_index) VALUES ";
        inv = std::stoi(inventory);
        // combine inv values to the insertion query
        for (int i = 0; i < inv; i++) {
            sql_query += "('" + category + "', '" + cate_index + "'),";
        }
        // erase last ,
        sql_query.erase(sql_query.size() - 1, 1);
        db->update(sql_query);
        sql_query = "SELECT id FROM book_item WHERE category='" + category + "' AND cate_index='" + cate_index + "'";
        int num_rows = db->select(sql_query);
        auto result = db->result();
        db->free();
        std::cout << "Book is added successfully!" << std::endl;
        std::cout << "ID of inserted books are: ";
        std::string inserted = "";
        // if insertion is successful, query insertion id and print on the screen
        for (auto row = result.begin(); row < result.end(); row++) {
            inserted += row->row[0];
            if (row + 1 != result.end()) {
                inserted += ",";
            }
        }
        std::cout << inserted << std::endl;
    } else if (affected != -1) {
        std::cout << "Insertion failed, please try again.";
    }
    std::cout << std::endl;
}

void Library::deleteBook() {
    std::string bookID = "default";
    // loop until bookID is integer
    while (!db->isInt(bookID)) {
        std::cout << "Please input the book ID: ";
        std::cin >> bookID;
        db->escape(bookID);
    }
    std::string sql_query = "SELECT id FROM book_item WHERE id=" + bookID;
    // validate affected rows
    int num_rows = db->select(sql_query);
    try {
        if (num_rows == 1) {
            sql_query = "DELETE FROM book_item WHERE id=" + bookID;
            db->update(sql_query);
            std::cout << "Book deleted." << std::endl;
        } else {
            std::cout << "Book does not exist." << std::endl;
        }
    } catch (std::exception &e) {
        // if book is borrowed, an exception will be thrown
        std::cout << "Someone has borrowed the book." << std::endl;
    }
}

void Library::circulateBook() {
    int op;
    bool exit = false;
    while (!exit) {
        std::vector<std::string> dops = {
                "Borrow Book",
                "Return Book",
                "Return to previous"
        };
        mkPanel("Circulate Books", dops);
        std::cout << "Input option No: ";
        std::cin >> op;
        switch (op) {
            case 1:
                borrowBook();
                break;
            case 2:
                returnBook();
                break;
            case 3:
                exit = true;
                break;
            default:
                std::cout << "Invalid opcode." << std::endl;
                // clear cin if it is a illness input
                if (std::cin.fail()) { std::cin.clear(); }
                break;
        }
    }
}

void Library::borrowBook() {
    std::string stuid = "default", bookID = "default";
    if (user.authLevel == 9) {
        while (!db->isInt(stuid)) {
            std::cout << "User ID: ";
            std::cin >> stuid;
        }
    } else {
        stuid = user.stuid;
    }
    while (!db->isInt(bookID)) {
        std::cout << "Book ID: ";
        std::cin >> bookID;
    }
    std::string sql_query = "INSERT INTO borrow (stuid, bookid) VALUES ('" + stuid + "', '" + bookID + "')";
    bool err_caught = false;
    try {
        db->update(sql_query);
    } catch (std::exception &e) {
        err_caught = true;
        std::string what = e.what();
        // if the exception is specified by trigger, print directly
        if (what[0] == '!') {
            what.erase(0, 1);
            std::cout << what << std::endl;
        } else {
            // else print generalized exception
            std::cout << "An error occured, please try again." << std::endl;
        }
    }
    if (!err_caught) {
        std::cout << "Book borrowed successfully" << std::endl;
    }
}

void Library::returnBook() {
    std::string stuid = "default", bookID = "default";
    if (user.authLevel == 9) {
        while (!db->isInt(stuid)) {
            std::cout << "User ID: ";
            std::cin >> stuid;
        }
    } else {
        stuid = user.stuid;
    }
    while (!db->isInt(bookID)) {
        std::cout << "Book ID: ";
        std::cin >> bookID;
    }
    std::string sql_query =
            "UPDATE borrow SET status=0 WHERE stuid='" + stuid + "' AND bookid='" + bookID + "' AND status=1";
    bool err_caught = false;
    long affected_rows;
    try {
        affected_rows = db->update(sql_query);
    } catch (std::exception &e) {
        err_caught = true;
        std::string what = e.what();
        // also, specified error is controlled by DB
        if (what[0] == '!') {
            what.erase(0, 1);
            std::cout << what << std::endl;
        } else {
            std::cout << "An error occured, please try again." << std::endl;
        }
    }
    if (!err_caught) {
        if (affected_rows > 0) {
            std::cout << "Book returned successfully." << std::endl;
        } else {
            std::cout << "An error occurred, you might enter a wrong bookid." << std::endl;
        }
    }
}

void Library::doLogOut() {
    // set loggedIn status to false to log out user
    user.loggedIn = false;
}
