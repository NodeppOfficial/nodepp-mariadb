# NODEPP-MySQL
Run **MySQL** in Nodepp

## Dependencies
- libmysqlclient-dev
  - 🪟: `pacman -S mingw-w64-x86_64-libmariadbclient`
  - 🐧: `sudo apt install libmysqlclient-dev`

## Example
```cpp
#include <nodepp/nodepp.h>
#include <mysql.h>

using namespace nodepp;

void onMain() {

    mysql_t db ("mysql://usr:pass@localhost:8000","dbName");

    db.exec(R"(
        CREATE TABLE COMPANY(
        ID INT PRIMARY KEY     NOT NULL,
        NAME           TEXT    NOT NULL,
        AGE            INT     NOT NULL,
        ADDRESS        CHAR(50),
        SALARY         REAL );
    )");

    db.exec(R"(
        INSERT INTO COMPANY ( ID, NAME, AGE, ADDRESS, SALARY )
        VALUES (1, 'Paul', 32, 'California', 20000.00 );
    )");

    db.exec(R"(
        INSERT INTO COMPANY ( ID, NAME, AGE, ADDRESS, SALARY )
        VALUES (2, 'John', 32, 'California', 20000.00 );
    )");

    db.exec(R"(
        INSERT INTO COMPANY ( ID, NAME, AGE, ADDRESS, SALARY )
        VALUES (3, 'Mery', 32, 'California', 20000.00 );
    )");

    db.exec(R"(
        INSERT INTO COMPANY ( ID, NAME, AGE, ADDRESS, SALARY )
        VALUES (4, 'Pipi', 32, 'California', 20000.00 );
    )");

    db.exec("SELECT * from COMPANY",[]( object_t args ){
        for( auto &x: args.keys() ){
             console::log( x, "->", args[x].as<string_t>() );
        }
    });

}
```

## Compilation
`g++ -o main main.cpp -I ./include -lmysqlclient ; ./main`
