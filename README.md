# NODEPP-MARIADB
Run **MariaDB** in Nodepp

## Dependencies

- libmariadbclient-dev
  - 🪟: `pacman -S mingw-w64-x86_64-libmariadbclient`
  - 🐧: `sudo apt install libmariadb-dev`

- Openssl
  - 🪟: `pacman -S mingw-w64-ucrt-x86_64-openssl`
  - 🐧: `sudo apt install libssl-dev`

## Example
```cpp
#include <nodepp/nodepp.h>
#include <mariadb.h>

using namespace nodepp;

void onMain() {

    mariadb_t db ("db://usr:pass@localhost:8000","dbName");

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

    db.exec("SELECT * from COMPANY",[]( sql_item_t args ){
        for( auto &x: args.keys() ){
             console::log( x, "->", args[x] );
        }
    });

}
```

## Compilation
`g++ -o main main.cpp -I ./include -lmariadb -lssl -lcrypto ; ./main`
