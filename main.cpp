#include "Library.h"

int main() {
    try {
        Library *lib;
        MyDatabase *db;
        db = new MyDatabase("localhost", "library", "default", "library");
        lib = new Library(db);
        lib->run();
    } catch (std::exception &e) {
        std::cout << "Exception caught: " << e.what() << std::endl;
    }
    return 0;
}
