# Nodepp MariaDB/MySQL Client

A high-performance, asynchronous client wrapper for MariaDB and MySQL, built using the Nodepp framework and the standard libmysqlclient C API. It converts the synchronous nature of the C API into efficient, non-blocking operations via coroutines and promises.

## Dependencies & CMake Integration
```bash
#libmariadbclient-dev
  🪟: pacman -S mingw-w64-x86_64-libmariadbclient
  🐧: sudo apt install libmariadb-dev
#Openssl
  🪟: pacman -S mingw-w64-ucrt-x86_64-openssl
  🐧: sudo apt install libssl-dev
```
```bash
include(FetchContent)

FetchContent_Declare(
	nodepp
	GIT_REPOSITORY   https://github.com/NodeppOfficial/nodepp
	GIT_TAG          origin/main
	GIT_PROGRESS     ON
)
FetchContent_MakeAvailable(nodepp)

FetchContent_Declare(
	nodepp-mariadb
	GIT_REPOSITORY   https://github.com/NodeppOfficial/nodepp-mariadb
	GIT_TAG          origin/main
	GIT_PROGRESS     ON
)
FetchContent_MakeAvailable(nodepp-mariadb)

#[...]

target_link_libraries( #[...]
	PUBLIC nodepp nodepp-mariadb #[...]
)
```

## Connection and Initialization
The mariadb_t object is constructed using a standard connection URI and the target database name.

```cpp
#include <nodepp/nodepp.h>
#include <mariadb/mariadb.h>

using namespace nodepp;

void main() {
try {
    // Connect to 'mydb' using details from the URI (e.g., 'mysql://user:pass@host:3306')
    auto db = mariadb_t("mysql://localhost:3306", "mydb");

    // Example: Execute a non-query command synchronously
    db.await("CREATE TABLE IF NOT EXISTS items (id INT PRIMARY KEY AUTO_INCREMENT, name TEXT);");

    console::log("Connected and table created successfully.");

} catch( except_t error ) {
    console::error("Connection failed:", error.what());
}}
```

## Asynchronous API Reference

The client offers three synchronized methods for executing SQL, ensuring seamless integration into Nodepp's event-driven environment.

**1. Asynchronous with Promise (.resolve())**

This is the preferred method for general queries (SELECT). It runs the query in the background and returns a promise that resolves with the entire result set upon completion.

```cpp
db.resolve("SELECT name FROM users WHERE age > 25;")
    
.then([]( array_t<sql_item_t> results ) {
    console::log("Found %d users.", results.size());
    // Results are mapped using column names as keys
})

.fail([]( except_t error ) {
    console::error("Async query failed:", error.what());
});
```

**2. Synchronous/Blocking (.await())**

Use this when you need an immediate result in a fiber context. It provides a simple, synchronous interface by internally awaiting the result of the promise.

```cpp
try {

    auto results = db.await("SELECT COUNT(*) AS total FROM items;");
    console::log("Total items:", results[0]["total"]);

} catch( except_t error ) {
    console::error("Synchronous query failed:", error.what());
}
```

**3. Asynchronous Streaming/Fire-and-Forget (.emit())**

Use emit() for non-query commands (INSERT, UPDATE, DELETE) or when processing very large result sets row-by-row via a callback.

```cpp
// Non-query command. Executes successfully in the background.
db.emit("INSERT INTO items (name) VALUES ('Hammer');");
db.emit("UPDATE items SET price = 100 WHERE id = 5;");
console::log("Database modifications dispatched.");
```

```cpp
// Example: Streaming results
db.emit("SELECT * FROM huge_log_table;", []( sql_item_t row ) {
    // Process the row immediately as it's fetched from the server.
    if (row["status"] == "ERROR") {
        console::warn("Found log error ID:", row["id"]);
    }
});
```

## Compilation
```bash
g++ -o main main.cpp -I ./include -lmariadb -lssl -lcrypto ; ./main
```

## License
**Nodepp-MariaDB** is distributed under the MIT License. See the LICENSE file for more details.
