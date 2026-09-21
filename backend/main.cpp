#include <drogon/drogon.h>
#include <sqlite3.h>
#include <iostream>
#include <string>
#include <cstdlib>

using namespace drogon;

// =========================================================
// SQL HELPER
// =========================================================

bool executeSQL(sqlite3 *db, const char *sql)
{
    char *errorMessage = nullptr;

    int result = sqlite3_exec(
        db,
        sql,
        nullptr,
        nullptr,
        &errorMessage
    );

    if (result != SQLITE_OK)
    {
        std::cerr << "SQL error: "
                  << (errorMessage ? errorMessage : "")
                  << "\n";

        if (errorMessage)
            sqlite3_free(errorMessage);

        return false;
    }

    return true;
}

// =========================================================
// MAIN
// =========================================================

int main()
{
    sqlite3 *db = nullptr;

    int result = sqlite3_open("vinothmart.db", &db);

    if (result != SQLITE_OK)
    {
        std::cerr << "Database opening failed!\n";
        return 1;
    }

    std::cout
        << "SQLite database connected successfully!\n";

    // =====================================================
    // USERS TABLE
    // =====================================================

    const char *usersTable = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            email TEXT UNIQUE NOT NULL,
            password TEXT NOT NULL,
            role TEXT NOT NULL
        );
    )";

    // =====================================================
    // PRODUCTS TABLE
    // =====================================================

    const char *productsTable = R"(
        CREATE TABLE IF NOT EXISTS products (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            seller_id INTEGER,
            name TEXT NOT NULL,
            price REAL NOT NULL,
            quantity INTEGER NOT NULL,
            category TEXT,
            FOREIGN KEY (seller_id)
                REFERENCES users(id)
        );
    )";

    // =====================================================
    // ORDERS TABLE
    // =====================================================

    const char *ordersTable = R"(
        CREATE TABLE IF NOT EXISTS orders (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            buyer_id INTEGER,
            customer_name TEXT NOT NULL,
            mobile TEXT,
            address TEXT,
            city TEXT,
            pincode TEXT,
            payment TEXT,
            total REAL NOT NULL,
            status TEXT NOT NULL,
            order_date TEXT,
            FOREIGN KEY (buyer_id)
                REFERENCES users(id)
        );
    )";

    // =====================================================
    // ORDER ITEMS TABLE
    // =====================================================

    const char *orderItemsTable = R"(
        CREATE TABLE IF NOT EXISTS order_items (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            order_id INTEGER NOT NULL,
            product_id INTEGER,
            product_name TEXT NOT NULL,
            price REAL NOT NULL,
            quantity INTEGER NOT NULL,
            FOREIGN KEY (order_id)
                REFERENCES orders(id),
            FOREIGN KEY (product_id)
                REFERENCES products(id)
        );
    )";

    // =====================================================
    // REVIEWS TABLE
    // =====================================================

    const char *reviewsTable = R"(
        CREATE TABLE IF NOT EXISTS reviews (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            product_id INTEGER,
            buyer_id INTEGER,
            rating INTEGER NOT NULL,
            review_text TEXT NOT NULL,
            review_date TEXT,
            FOREIGN KEY (product_id)
                REFERENCES products(id),
            FOREIGN KEY (buyer_id)
                REFERENCES users(id)
        );
    )";

    // =====================================================
    // CART TABLE
    // =====================================================

    const char *cartTable = R"(
        CREATE TABLE IF NOT EXISTS cart_items (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            buyer_id INTEGER NOT NULL,
            product_id INTEGER NOT NULL,
            quantity INTEGER NOT NULL DEFAULT 1,
            UNIQUE(buyer_id, product_id)
        );
    )";

    // =====================================================
    // CREATE TABLES
    // =====================================================

    if (!executeSQL(db, usersTable))
    {
        sqlite3_close(db);
        return 1;
    }

    if (!executeSQL(db, productsTable))
    {
        sqlite3_close(db);
        return 1;
    }

    if (!executeSQL(db, ordersTable))
    {
        sqlite3_close(db);
        return 1;
    }

    if (!executeSQL(db, orderItemsTable))
    {
        sqlite3_close(db);
        return 1;
    }

    if (!executeSQL(db, reviewsTable))
    {
        sqlite3_close(db);
        return 1;
    }

    if (!executeSQL(db, cartTable))
    {
        sqlite3_close(db);
        return 1;
    }

    std::cout
        << "All database tables are ready!\n";

    // =====================================================
    // GLOBAL CORS
    // =====================================================

    app().registerPreRoutingAdvice(
        [](const HttpRequestPtr &req,
           AdviceCallback &&callback,
           AdviceChainCallback &&chainCallback)
        {
            if (req->method() == Options)
            {
                auto response =
                    HttpResponse::newHttpResponse();

                response->setStatusCode(k200OK);

                response->addHeader(
                    "Access-Control-Allow-Origin",
                    "*"
                );

                response->addHeader(
                    "Access-Control-Allow-Methods",
                    "GET, POST, PUT, DELETE, OPTIONS"
                );

                response->addHeader(
                    "Access-Control-Allow-Headers",
                    "Content-Type"
                );

                callback(response);
                return;
            }

            chainCallback();
        }
    );

    app().registerPreSendingAdvice(
        [](const HttpRequestPtr &req,
           const HttpResponsePtr &response)
        {
            response->addHeader(
                "Access-Control-Allow-Origin",
                "*"
            );

            response->addHeader(
                "Access-Control-Allow-Methods",
                "GET, POST, PUT, DELETE, OPTIONS"
            );

            response->addHeader(
                "Access-Control-Allow-Headers",
                "Content-Type"
            );
        }
    );

    // =====================================================
    // ROOT API
    // =====================================================

    app().registerHandler(
        "/",
        [](const HttpRequestPtr &req,
           std::function<void(
               const HttpResponsePtr &)> &&callback)
        {
            auto response =
                HttpResponse::newHttpResponse();

            response->setBody(
                "Vinoth Mart Backend + SQLite is Running!"
            );

            callback(response);
        },
        {Get}
    );

    // =====================================================
    // REGISTER API
    // =====================================================

    app().registerHandler(
        "/api/register",
        [db](
            const HttpRequestPtr &req,
            std::function<void(
                const HttpResponsePtr &)> &&callback)
        {
            auto json = req->getJsonObject();

            Json::Value result;

            if (!json)
            {
                result["success"] = false;
                result["message"] =
                    "Invalid JSON data.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            std::string name =
                (*json)["name"].asString();

            std::string email =
                (*json)["email"].asString();

            std::string password =
                (*json)["password"].asString();

            std::string role =
                (*json)["role"].asString();

            if (
                name.empty() ||
                email.empty() ||
                password.empty() ||
                role.empty()
            )
            {
                result["success"] = false;
                result["message"] =
                    "All fields are required.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            if (
                role != "buyer" &&
                role != "seller"
            )
            {
                result["success"] = false;
                result["message"] =
                    "Invalid role.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            sqlite3_stmt *stmt = nullptr;

            const char *sql = R"(
                INSERT INTO users
                (name, email, password, role)
                VALUES (?, ?, ?, ?);
            )";

            if (
                sqlite3_prepare_v2(
                    db,
                    sql,
                    -1,
                    &stmt,
                    nullptr
                ) != SQLITE_OK
            )
            {
                result["success"] = false;
                result["message"] =
                    "Database error.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            sqlite3_bind_text(
                stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT
            );

            sqlite3_bind_text(
                stmt, 2, email.c_str(), -1, SQLITE_TRANSIENT
            );

            sqlite3_bind_text(
                stmt, 3, password.c_str(), -1, SQLITE_TRANSIENT
            );

            sqlite3_bind_text(
                stmt, 4, role.c_str(), -1, SQLITE_TRANSIENT
            );

            int stepResult =
                sqlite3_step(stmt);

            if (stepResult == SQLITE_DONE)
            {
                result["success"] = true;
                result["message"] =
                    "Registration successful!";
            }
            else if (stepResult == SQLITE_CONSTRAINT)
            {
                result["success"] = false;
                result["message"] =
                    "Email already registered.";
            }
            else
            {
                result["success"] = false;
                result["message"] =
                    "Registration failed.";
            }

            sqlite3_finalize(stmt);

            callback(
                HttpResponse::newHttpJsonResponse(result)
            );
        },
        {Post}
    );

    // =====================================================
    // LOGIN API
    // =====================================================

    app().registerHandler(
        "/api/login",
        [db](
            const HttpRequestPtr &req,
            std::function<void(
                const HttpResponsePtr &)> &&callback)
        {
            auto json = req->getJsonObject();

            Json::Value result;

            if (!json)
            {
                result["success"] = false;
                result["message"] =
                    "Invalid JSON data.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            std::string email =
                (*json)["email"].asString();

            std::string password =
                (*json)["password"].asString();

            std::string role =
                (*json)["role"].asString();

            sqlite3_stmt *stmt = nullptr;

            const char *sql = R"(
                SELECT id, name, email, role
                FROM users
                WHERE email = ?
                AND password = ?
                AND role = ?;
            )";

            if (
                sqlite3_prepare_v2(
                    db,
                    sql,
                    -1,
                    &stmt,
                    nullptr
                ) != SQLITE_OK
            )
            {
                result["success"] = false;
                result["message"] =
                    "Database error.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            sqlite3_bind_text(
                stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT
            );

            sqlite3_bind_text(
                stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT
            );

            sqlite3_bind_text(
                stmt, 3, role.c_str(), -1, SQLITE_TRANSIENT
            );

            int stepResult =
                sqlite3_step(stmt);

            if (stepResult == SQLITE_ROW)
            {
                result["success"] = true;
                result["message"] =
                    "Login successful!";

                result["user"]["id"] =
                    sqlite3_column_int(stmt, 0);

                const char *name =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(stmt, 1)
                    );

                const char *userEmail =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(stmt, 2)
                    );

                const char *userRole =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(stmt, 3)
                    );

                result["user"]["name"] =
                    name ? name : "";

                result["user"]["email"] =
                    userEmail ? userEmail : "";

                result["user"]["role"] =
                    userRole ? userRole : "";
            }
            else
            {
                result["success"] = false;
                result["message"] =
                    "Invalid email or password.";
            }

            sqlite3_finalize(stmt);

            callback(
                HttpResponse::newHttpJsonResponse(result)
            );
        },
        {Post}
    );

    // =====================================================
    // GET PRODUCTS
    // =====================================================

    app().registerHandler(
        "/api/products",
        [db](
            const HttpRequestPtr &req,
            std::function<void(
                const HttpResponsePtr &)> &&callback)
        {
            sqlite3_stmt *stmt = nullptr;

            const char *sql = R"(
                SELECT
                    id,
                    seller_id,
                    name,
                    price,
                    quantity,
                    category
                FROM products
                ORDER BY id DESC;
            )";

            Json::Value result;

            result["success"] = true;
            result["products"] =
                Json::arrayValue;

            if (
                sqlite3_prepare_v2(
                    db,
                    sql,
                    -1,
                    &stmt,
                    nullptr
                ) != SQLITE_OK
            )
            {
                result["success"] = false;
                result["message"] =
                    "Failed to load products.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            while (
                sqlite3_step(stmt)
                == SQLITE_ROW
            )
            {
                Json::Value product;

                product["id"] =
                    sqlite3_column_int(stmt, 0);

                product["seller_id"] =
                    sqlite3_column_int(stmt, 1);

                const char *name =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(stmt, 2)
                    );

                product["name"] =
                    name ? name : "";

                product["price"] =
                    sqlite3_column_double(stmt, 3);

                product["quantity"] =
                    sqlite3_column_int(stmt, 4);

                const char *category =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(stmt, 5)
                    );

                product["category"] =
                    category ? category : "";

                result["products"].append(product);
            }

            sqlite3_finalize(stmt);

            callback(
                HttpResponse::newHttpJsonResponse(result)
            );
        },
        {Get}
    );

    // =====================================================
    // ADD PRODUCT
    // =====================================================

    app().registerHandler(
        "/api/products",
        [db](
            const HttpRequestPtr &req,
            std::function<void(
                const HttpResponsePtr &)> &&callback)
        {
            auto json = req->getJsonObject();

            Json::Value result;

            if (!json)
            {
                result["success"] = false;
                result["message"] =
                    "Invalid JSON data.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            int sellerId =
                (*json)["seller_id"].asInt();

            std::string name =
                (*json)["name"].asString();

            double price =
                (*json)["price"].asDouble();

            int quantity =
                (*json)["quantity"].asInt();

            std::string category =
                (*json)["category"].asString();

            if (
                name.empty() ||
                price <= 0 ||
                quantity < 0
            )
            {
                result["success"] = false;
                result["message"] =
                    "Invalid product data.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            sqlite3_stmt *stmt = nullptr;

            const char *sql = R"(
                INSERT INTO products
                (
                    seller_id,
                    name,
                    price,
                    quantity,
                    category
                )
                VALUES (?, ?, ?, ?, ?);
            )";

            if (
                sqlite3_prepare_v2(
                    db,
                    sql,
                    -1,
                    &stmt,
                    nullptr
                ) != SQLITE_OK
            )
            {
                result["success"] = false;
                result["message"] =
                    "Database error.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            sqlite3_bind_int(stmt, 1, sellerId);

            sqlite3_bind_text(
                stmt, 2, name.c_str(), -1, SQLITE_TRANSIENT
            );

            sqlite3_bind_double(stmt, 3, price);

            sqlite3_bind_int(stmt, 4, quantity);

            sqlite3_bind_text(
                stmt, 5, category.c_str(), -1, SQLITE_TRANSIENT
            );

            int stepResult =
                sqlite3_step(stmt);

            if (stepResult == SQLITE_DONE)
            {
                result["success"] = true;
                result["message"] =
                    "Product added successfully!";

                result["id"] =
                    static_cast<Json::Int64>(
                        sqlite3_last_insert_rowid(db)
                    );
            }
            else
            {
                result["success"] = false;
                result["message"] =
                    "Failed to add product.";
            }

            sqlite3_finalize(stmt);

            callback(
                HttpResponse::newHttpJsonResponse(result)
            );
        },
        {Post}
    );

    // =====================================================
    // UPDATE PRODUCT
    // =====================================================

    app().registerHandler(
        "/api/products/update",
        [db](
            const HttpRequestPtr &req,
            std::function<void(
                const HttpResponsePtr &)> &&callback)
        {
            auto json = req->getJsonObject();

            Json::Value result;

            if (!json)
            {
                result["success"] = false;
                result["message"] =
                    "Invalid JSON data.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            long long id =
                (*json)["id"].asInt64();

            int sellerId =
                (*json)["seller_id"].asInt();

            std::string name =
                (*json)["name"].asString();

            double price =
                (*json)["price"].asDouble();

            int quantity =
                (*json)["quantity"].asInt();

            std::string category =
                (*json)["category"].asString();

            if (
                id <= 0 ||
                sellerId <= 0 ||
                name.empty() ||
                price <= 0 ||
                quantity < 0
            )
            {
                result["success"] = false;
                result["message"] =
                    "Invalid product data.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            sqlite3_stmt *stmt = nullptr;

            const char *sql = R"(
                UPDATE products
                SET
                    name = ?,
                    price = ?,
                    quantity = ?,
                    category = ?,
                    seller_id = ?
                WHERE id = ?
            )";

            if (
                sqlite3_prepare_v2(
                    db,
                    sql,
                    -1,
                    &stmt,
                    nullptr
                ) != SQLITE_OK
            )
            {
                result["success"] = false;
                result["message"] =
                    "Database error.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            sqlite3_bind_text(
                stmt,
                1,
                name.c_str(),
                -1,
                SQLITE_TRANSIENT
            );

            sqlite3_bind_double(
                stmt,
                2,
                price
            );

            sqlite3_bind_int(
                stmt,
                3,
                quantity
            );

            sqlite3_bind_text(
                stmt,
                4,
                category.c_str(),
                -1,
                SQLITE_TRANSIENT
            );

            sqlite3_bind_int(
                stmt,
                5,
                sellerId
            );

            sqlite3_bind_int64(
                stmt,
                6,
                id
            );

            int stepResult =
                sqlite3_step(stmt);

            if (
                stepResult == SQLITE_DONE &&
                sqlite3_changes(db) > 0
            )
            {
                result["success"] = true;
                result["message"] =
                    "Product updated successfully!";
            }
            else
            {
                result["success"] = false;
                result["message"] =
                    "Product not found or update failed.";
            }

            sqlite3_finalize(stmt);

            callback(
                HttpResponse::newHttpJsonResponse(result)
            );
        },
        {Put}
    );

    // =====================================================
    // DELETE PRODUCT
    // =====================================================

    app().registerHandler(
        "/api/products/delete",
        [db](
            const HttpRequestPtr &req,
            std::function<void(
                const HttpResponsePtr &)> &&callback)
        {
            auto json = req->getJsonObject();

            Json::Value result;

            if (!json)
            {
                result["success"] = false;
                result["message"] =
                    "Invalid JSON data.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            long long id =
                (*json)["id"].asInt64();

            sqlite3_stmt *stmt = nullptr;

            const char *sql =
                "DELETE FROM products WHERE id = ?;";

            if (
                sqlite3_prepare_v2(
                    db,
                    sql,
                    -1,
                    &stmt,
                    nullptr
                ) != SQLITE_OK
            )
            {
                result["success"] = false;
                result["message"] =
                    "Database error.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            sqlite3_bind_int64(
                stmt,
                1,
                id
            );

            int stepResult =
                sqlite3_step(stmt);

            if (
                stepResult == SQLITE_DONE &&
                sqlite3_changes(db) > 0
            )
            {
                result["success"] = true;
                result["message"] =
                    "Product deleted successfully!";
            }
            else
            {
                result["success"] = false;
                result["message"] =
                    "Product not found.";
            }

            sqlite3_finalize(stmt);

            callback(
                HttpResponse::newHttpJsonResponse(result)
            );
        },
        {Delete}
    );

    // =====================================================
    // GET CART
    // =====================================================

    app().registerHandler(
        "/api/cart",
        [db](
            const HttpRequestPtr &req,
            std::function<void(
                const HttpResponsePtr &)> &&callback)
        {
            std::string buyerIdText =
                req->getParameter("buyer_id");

            Json::Value result;

            result["success"] = true;
            result["cart"] =
                Json::arrayValue;

            if (buyerIdText.empty())
            {
                result["success"] = false;
                result["message"] =
                    "buyer_id is required.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            int buyerId =
                std::stoi(buyerIdText);

            sqlite3_stmt *stmt = nullptr;

            const char *sql = R"(
                SELECT
                    c.id,
                    c.product_id,
                    p.name,
                    p.price,
                    p.quantity,
                    p.category,
                    c.quantity
                FROM cart_items c
                JOIN products p
                    ON c.product_id = p.id
                WHERE c.buyer_id = ?;
            )";

            if (
                sqlite3_prepare_v2(
                    db,
                    sql,
                    -1,
                    &stmt,
                    nullptr
                ) != SQLITE_OK
            )
            {
                result["success"] = false;
                result["message"] =
                    "Failed to load cart.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            sqlite3_bind_int(
                stmt,
                1,
                buyerId
            );

            while (
                sqlite3_step(stmt)
                == SQLITE_ROW
            )
            {
                Json::Value item;

                item["id"] =
                    sqlite3_column_int(stmt, 0);

                item["product_id"] =
                    sqlite3_column_int(stmt, 1);

                const char *name =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(stmt, 2)
                    );

                item["name"] =
                    name ? name : "";

                item["price"] =
                    sqlite3_column_double(stmt, 3);

                item["available_quantity"] =
                    sqlite3_column_int(stmt, 4);

                const char *category =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(stmt, 5)
                    );

                item["category"] =
                    category ? category : "";

                item["quantity"] =
                    sqlite3_column_int(stmt, 6);

                result["cart"].append(item);
            }

            sqlite3_finalize(stmt);

            callback(
                HttpResponse::newHttpJsonResponse(result)
            );
        },
        {Get}
    );

    // =====================================================
    // ADD TO CART
    // =====================================================

    app().registerHandler(
        "/api/cart",
        [db](
            const HttpRequestPtr &req,
            std::function<void(
                const HttpResponsePtr &)> &&callback)
        {
            auto json = req->getJsonObject();

            Json::Value result;

            if (!json)
            {
                result["success"] = false;
                result["message"] =
                    "Invalid JSON data.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            int buyerId =
                (*json)["buyer_id"].asInt();

            int productId =
                (*json)["product_id"].asInt();

            int quantity =
                (*json)["quantity"].asInt();

            if (quantity <= 0)
                quantity = 1;

            sqlite3_stmt *productStmt = nullptr;

            const char *productSQL = R"(
                SELECT quantity
                FROM products
                WHERE id = ?;
            )";

            if (
                sqlite3_prepare_v2(
                    db,
                    productSQL,
                    -1,
                    &productStmt,
                    nullptr
                ) != SQLITE_OK
            )
            {
                result["success"] = false;
                result["message"] =
                    "Database error.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            sqlite3_bind_int(
                productStmt,
                1,
                productId
            );

            int stock = -1;

            if (
                sqlite3_step(productStmt)
                == SQLITE_ROW
            )
            {
                stock =
                    sqlite3_column_int(
                        productStmt,
                        0
                    );
            }

            sqlite3_finalize(productStmt);

            if (stock < 0)
            {
                result["success"] = false;
                result["message"] =
                    "Product not found.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            sqlite3_stmt *checkStmt = nullptr;

            const char *checkSQL = R"(
                SELECT quantity
                FROM cart_items
                WHERE buyer_id = ?
                AND product_id = ?;
            )";

            sqlite3_prepare_v2(
                db,
                checkSQL,
                -1,
                &checkStmt,
                nullptr
            );

            sqlite3_bind_int(
                checkStmt,
                1,
                buyerId
            );

            sqlite3_bind_int(
                checkStmt,
                2,
                productId
            );

            int existingQuantity = 0;

            if (
                sqlite3_step(checkStmt)
                == SQLITE_ROW
            )
            {
                existingQuantity =
                    sqlite3_column_int(
                        checkStmt,
                        0
                    );
            }

            sqlite3_finalize(checkStmt);

            int newQuantity =
                existingQuantity + quantity;

            if (newQuantity > stock)
            {
                result["success"] = false;
                result["message"] =
                    "Not enough stock available.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            sqlite3_stmt *stmt = nullptr;

            const char *sql = R"(
                INSERT INTO cart_items
                (buyer_id, product_id, quantity)
                VALUES (?, ?, ?)
                ON CONFLICT(buyer_id, product_id)
                DO UPDATE SET
                    quantity = excluded.quantity;
            )";

            sqlite3_prepare_v2(
                db,
                sql,
                -1,
                &stmt,
                nullptr
            );

            sqlite3_bind_int(stmt, 1, buyerId);
            sqlite3_bind_int(stmt, 2, productId);
            sqlite3_bind_int(stmt, 3, newQuantity);

            if (
                sqlite3_step(stmt)
                == SQLITE_DONE
            )
            {
                result["success"] = true;
                result["message"] =
                    "Added to cart successfully!";
            }
            else
            {
                result["success"] = false;
                result["message"] =
                    "Failed to add to cart.";
            }

            sqlite3_finalize(stmt);

            callback(
                HttpResponse::newHttpJsonResponse(result)
            );
        },
        {Post}
    );

    // =====================================================
    // UPDATE CART
    // =====================================================

    app().registerHandler(
        "/api/cart",
        [db](
            const HttpRequestPtr &req,
            std::function<void(
                const HttpResponsePtr &)> &&callback)
        {
            auto json = req->getJsonObject();

            Json::Value result;

            if (!json)
            {
                result["success"] = false;
                result["message"] =
                    "Invalid JSON data.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            int buyerId =
                (*json)["buyer_id"].asInt();

            int productId =
                (*json)["product_id"].asInt();

            int quantity =
                (*json)["quantity"].asInt();

            if (quantity <= 0)
            {
                result["success"] = false;
                result["message"] =
                    "Invalid quantity.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            sqlite3_stmt *stockStmt = nullptr;

            sqlite3_prepare_v2(
                db,
                "SELECT quantity FROM products WHERE id = ?;",
                -1,
                &stockStmt,
                nullptr
            );

            sqlite3_bind_int(
                stockStmt,
                1,
                productId
            );

            int stock = -1;

            if (
                sqlite3_step(stockStmt)
                == SQLITE_ROW
            )
            {
                stock =
                    sqlite3_column_int(
                        stockStmt,
                        0
                    );
            }

            sqlite3_finalize(stockStmt);

            if (
                stock < 0 ||
                quantity > stock
            )
            {
                result["success"] = false;
                result["message"] =
                    "Quantity exceeds available stock.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            sqlite3_stmt *stmt = nullptr;

            sqlite3_prepare_v2(
                db,
                "UPDATE cart_items "
                "SET quantity = ? "
                "WHERE buyer_id = ? "
                "AND product_id = ?;",
                -1,
                &stmt,
                nullptr
            );

            sqlite3_bind_int(stmt, 1, quantity);
            sqlite3_bind_int(stmt, 2, buyerId);
            sqlite3_bind_int(stmt, 3, productId);

            int stepResult =
                sqlite3_step(stmt);

            if (
                stepResult == SQLITE_DONE &&
                sqlite3_changes(db) > 0
            )
            {
                result["success"] = true;
                result["message"] =
                    "Cart updated successfully!";
            }
            else
            {
                result["success"] = false;
                result["message"] =
                    "Cart item not found.";
            }

            sqlite3_finalize(stmt);

            callback(
                HttpResponse::newHttpJsonResponse(result)
            );
        },
        {Put}
    );

    // =====================================================
    // DELETE CART ITEM
    // =====================================================

    app().registerHandler(
        "/api/cart",
        [db](
            const HttpRequestPtr &req,
            std::function<void(
                const HttpResponsePtr &)> &&callback)
        {
            auto json = req->getJsonObject();

            Json::Value result;

            if (!json)
            {
                result["success"] = false;
                result["message"] =
                    "Invalid JSON data.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            int buyerId =
                (*json)["buyer_id"].asInt();

            int productId =
                (*json)["product_id"].asInt();

            sqlite3_stmt *stmt = nullptr;

            sqlite3_prepare_v2(
                db,
                "DELETE FROM cart_items "
                "WHERE buyer_id = ? "
                "AND product_id = ?;",
                -1,
                &stmt,
                nullptr
            );

            sqlite3_bind_int(stmt, 1, buyerId);
            sqlite3_bind_int(stmt, 2, productId);

            int stepResult =
                sqlite3_step(stmt);

            if (
                stepResult == SQLITE_DONE &&
                sqlite3_changes(db) > 0
            )
            {
                result["success"] = true;
                result["message"] =
                    "Cart item removed successfully!";
            }
            else
            {
                result["success"] = false;
                result["message"] =
                    "Cart item not found.";
            }

            sqlite3_finalize(stmt);

            callback(
                HttpResponse::newHttpJsonResponse(result)
            );
        },
        {Delete}
    );

    // =====================================================
    // PLACE ORDER
    // =====================================================

    app().registerHandler(
        "/api/orders",
        [db](
            const HttpRequestPtr &req,
            std::function<void(
                const HttpResponsePtr &)> &&callback)
        {
            auto json = req->getJsonObject();

            Json::Value result;

            if (!json)
            {
                result["success"] = false;
                result["message"] =
                    "Invalid JSON data.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            int buyerId =
                (*json)["buyer_id"].asInt();

            double total =
                (*json)["total"].asDouble();

            std::string payment =
                (*json)["payment"].asString();

            std::string customerName =
                (*json)["customer_name"].asString();

            std::string mobile =
                (*json)["mobile"].asString();

            std::string address =
                (*json)["address"].asString();

            std::string city =
                (*json)["city"].asString();

            std::string pincode =
                (*json)["pincode"].asString();

            const Json::Value items =
                (*json)["items"];

            if (
                buyerId <= 0 ||
                total <= 0 ||
                !items.isArray() ||
                items.empty()
            )
            {
                result["success"] = false;
                result["message"] =
                    "Invalid order data.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            if (
                sqlite3_exec(
                    db,
                    "BEGIN TRANSACTION;",
                    nullptr,
                    nullptr,
                    nullptr
                ) != SQLITE_OK
            )
            {
                result["success"] = false;
                result["message"] =
                    "Could not start transaction.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            // INSERT ORDER

            sqlite3_stmt *orderStmt = nullptr;

            const char *orderSQL = R"(
                INSERT INTO orders
                (
                    buyer_id,
                    customer_name,
                    mobile,
                    address,
                    city,
                    pincode,
                    payment,
                    total,
                    status,
                    order_date
                )
                VALUES
                (
                    ?, ?, ?, ?, ?, ?,
                    ?, ?, 'Placed',
                    datetime('now')
                );
            )";

            if (
                sqlite3_prepare_v2(
                    db,
                    orderSQL,
                    -1,
                    &orderStmt,
                    nullptr
                ) != SQLITE_OK
            )
            {
                sqlite3_exec(
                    db,
                    "ROLLBACK;",
                    nullptr,
                    nullptr,
                    nullptr
                );

                result["success"] = false;
                result["message"] =
                    "Failed to create order.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            sqlite3_bind_int(orderStmt, 1, buyerId);

            sqlite3_bind_text(
                orderStmt,
                2,
                customerName.c_str(),
                -1,
                SQLITE_TRANSIENT
            );

            sqlite3_bind_text(
                orderStmt,
                3,
                mobile.c_str(),
                -1,
                SQLITE_TRANSIENT
            );

            sqlite3_bind_text(
                orderStmt,
                4,
                address.c_str(),
                -1,
                SQLITE_TRANSIENT
            );

            sqlite3_bind_text(
                orderStmt,
                5,
                city.c_str(),
                -1,
                SQLITE_TRANSIENT
            );

            sqlite3_bind_text(
                orderStmt,
                6,
                pincode.c_str(),
                -1,
                SQLITE_TRANSIENT
            );

            sqlite3_bind_text(
                orderStmt,
                7,
                payment.c_str(),
                -1,
                SQLITE_TRANSIENT
            );

            sqlite3_bind_double(
                orderStmt,
                8,
                total
            );

            if (
                sqlite3_step(orderStmt)
                != SQLITE_DONE
            )
            {
                sqlite3_finalize(orderStmt);

                sqlite3_exec(
                    db,
                    "ROLLBACK;",
                    nullptr,
                    nullptr,
                    nullptr
                );

                result["success"] = false;
                result["message"] =
                    "Failed to create order.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            sqlite3_finalize(orderStmt);

            long long orderId =
                sqlite3_last_insert_rowid(db);

            // INSERT ORDER ITEMS

            for (const auto &item : items)
            {
                int productId =
                    item["product_id"].asInt();

                int quantity =
                    item["quantity"].asInt();

                double price =
                    item["price"].asDouble();

                if (
                    productId <= 0 ||
                    quantity <= 0
                )
                {
                    sqlite3_exec(
                        db,
                        "ROLLBACK;",
                        nullptr,
                        nullptr,
                        nullptr
                    );

                    result["success"] = false;
                    result["message"] =
                        "Invalid order item.";

                    callback(
                        HttpResponse::newHttpJsonResponse(result)
                    );

                    return;
                }

                // GET PRODUCT

                sqlite3_stmt *productStmt = nullptr;

                sqlite3_prepare_v2(
                    db,
                    "SELECT name, quantity "
                    "FROM products "
                    "WHERE id = ?;",
                    -1,
                    &productStmt,
                    nullptr
                );

                sqlite3_bind_int(
                    productStmt,
                    1,
                    productId
                );

                int stock = -1;

                std::string productName =
                    "Product";

                if (
                    sqlite3_step(productStmt)
                    == SQLITE_ROW
                )
                {
                    const char *name =
                        reinterpret_cast<const char *>(
                            sqlite3_column_text(
                                productStmt,
                                0
                            )
                        );

                    if (name)
                        productName = name;

                    stock =
                        sqlite3_column_int(
                            productStmt,
                            1
                        );
                }

                sqlite3_finalize(productStmt);

                if (
                    stock < 0 ||
                    quantity > stock
                )
                {
                    sqlite3_exec(
                        db,
                        "ROLLBACK;",
                        nullptr,
                        nullptr,
                        nullptr
                    );

                    result["success"] = false;
                    result["message"] =
                        "Insufficient stock for " +
                        productName;

                    callback(
                        HttpResponse::newHttpJsonResponse(result)
                    );

                    return;
                }

                // SAVE ORDER ITEM

                sqlite3_stmt *itemStmt = nullptr;

                const char *itemSQL = R"(
                    INSERT INTO order_items
                    (
                        order_id,
                        product_id,
                        product_name,
                        price,
                        quantity
                    )
                    VALUES (?, ?, ?, ?, ?);
                )";

                sqlite3_prepare_v2(
                    db,
                    itemSQL,
                    -1,
                    &itemStmt,
                    nullptr
                );

                sqlite3_bind_int64(
                    itemStmt,
                    1,
                    orderId
                );

                sqlite3_bind_int(
                    itemStmt,
                    2,
                    productId
                );

                sqlite3_bind_text(
                    itemStmt,
                    3,
                    productName.c_str(),
                    -1,
                    SQLITE_TRANSIENT
                );

                sqlite3_bind_double(
                    itemStmt,
                    4,
                    price
                );

                sqlite3_bind_int(
                    itemStmt,
                    5,
                    quantity
                );

                if (
                    sqlite3_step(itemStmt)
                    != SQLITE_DONE
                )
                {
                    sqlite3_finalize(itemStmt);

                    sqlite3_exec(
                        db,
                        "ROLLBACK;",
                        nullptr,
                        nullptr,
                        nullptr
                    );

                    result["success"] = false;
                    result["message"] =
                        "Failed to save order item.";

                    callback(
                        HttpResponse::newHttpJsonResponse(result)
                    );

                    return;
                }

                sqlite3_finalize(itemStmt);

                // REDUCE STOCK

                sqlite3_stmt *stockUpdate = nullptr;

                sqlite3_prepare_v2(
                    db,
                    "UPDATE products "
                    "SET quantity = quantity - ? "
                    "WHERE id = ?;",
                    -1,
                    &stockUpdate,
                    nullptr
                );

                sqlite3_bind_int(
                    stockUpdate,
                    1,
                    quantity
                );

                sqlite3_bind_int(
                    stockUpdate,
                    2,
                    productId
                );

                sqlite3_step(stockUpdate);

                sqlite3_finalize(stockUpdate);
            }

            // CLEAR CART

            sqlite3_stmt *clearCart = nullptr;

            sqlite3_prepare_v2(
                db,
                "DELETE FROM cart_items "
                "WHERE buyer_id = ?;",
                -1,
                &clearCart,
                nullptr
            );

            sqlite3_bind_int(
                clearCart,
                1,
                buyerId
            );

            sqlite3_step(clearCart);

            sqlite3_finalize(clearCart);

            // COMMIT

            if (
                sqlite3_exec(
                    db,
                    "COMMIT;",
                    nullptr,
                    nullptr,
                    nullptr
                ) != SQLITE_OK
            )
            {
                result["success"] = false;
                result["message"] =
                    "Failed to complete order.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            result["success"] = true;

            result["message"] =
                "Order placed successfully!";

            result["order_id"] =
                static_cast<Json::Int64>(
                    orderId
                );

            result["buyer_id"] =
                buyerId;

            callback(
                HttpResponse::newHttpJsonResponse(result)
            );
        },
        {Post}
    );

    // =====================================================
    // BUYER ORDER HISTORY API
    // =====================================================

    app().registerHandler(
        "/api/orders/buyer",
        [db](
            const HttpRequestPtr &req,
            std::function<void(
                const HttpResponsePtr &)> &&callback)
        {
            Json::Value result;

            result["success"] = true;
            result["orders"] =
                Json::arrayValue;

            std::string buyerIdText =
                req->getParameter("buyer_id");

            if (buyerIdText.empty())
            {
                result["success"] = false;
                result["message"] =
                    "buyer_id is required.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            int buyerId =
                std::stoi(buyerIdText);

            sqlite3_stmt *orderStmt = nullptr;

            const char *orderSQL = R"(
                SELECT
                    id,
                    customer_name,
                    mobile,
                    address,
                    city,
                    pincode,
                    payment,
                    total,
                    status,
                    order_date
                FROM orders
                WHERE buyer_id = ?
                ORDER BY id DESC;
            )";

            if (
                sqlite3_prepare_v2(
                    db,
                    orderSQL,
                    -1,
                    &orderStmt,
                    nullptr
                ) != SQLITE_OK
            )
            {
                result["success"] = false;
                result["message"] =
                    "Failed to load orders.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            sqlite3_bind_int(
                orderStmt,
                1,
                buyerId
            );

            while (
                sqlite3_step(orderStmt)
                == SQLITE_ROW
            )
            {
                Json::Value order;

                int orderId =
                    sqlite3_column_int(
                        orderStmt,
                        0
                    );

                const char *customerName =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(
                            orderStmt,
                            1
                        )
                    );

                const char *mobile =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(
                            orderStmt,
                            2
                        )
                    );

                const char *address =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(
                            orderStmt,
                            3
                        )
                    );

                const char *city =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(
                            orderStmt,
                            4
                        )
                    );

                const char *pincode =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(
                            orderStmt,
                            5
                        )
                    );

                const char *payment =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(
                            orderStmt,
                            6
                        )
                    );

                double total =
                    sqlite3_column_double(
                        orderStmt,
                        7
                    );

                const char *status =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(
                            orderStmt,
                            8
                        )
                    );

                const char *orderDate =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(
                            orderStmt,
                            9
                        )
                    );

                order["id"] =
                    orderId;

                order["customer_name"] =
                    customerName ? customerName : "";

                order["mobile"] =
                    mobile ? mobile : "";

                order["address"] =
                    address ? address : "";

                order["city"] =
                    city ? city : "";

                order["pincode"] =
                    pincode ? pincode : "";

                order["payment"] =
                    payment ? payment : "";

                order["total"] =
                    total;

                order["status"] =
                    status ? status : "Placed";

                order["date"] =
                    orderDate ? orderDate : "";

                order["products"] =
                    Json::arrayValue;

                sqlite3_stmt *itemStmt = nullptr;

                const char *itemSQL = R"(
                    SELECT
                        product_id,
                        product_name,
                        price,
                        quantity
                    FROM order_items
                    WHERE order_id = ?;
                )";

                if (
                    sqlite3_prepare_v2(
                        db,
                        itemSQL,
                        -1,
                        &itemStmt,
                        nullptr
                    ) == SQLITE_OK
                )
                {
                    sqlite3_bind_int(
                        itemStmt,
                        1,
                        orderId
                    );

                    while (
                        sqlite3_step(itemStmt)
                        == SQLITE_ROW
                    )
                    {
                        Json::Value product;

                        product["product_id"] =
                            sqlite3_column_int(
                                itemStmt,
                                0
                            );

                        const char *productName =
                            reinterpret_cast<const char *>(
                                sqlite3_column_text(
                                    itemStmt,
                                    1
                                )
                            );

                        product["name"] =
                            productName
                                ? productName
                                : "Product";

                        product["price"] =
                            sqlite3_column_double(
                                itemStmt,
                                2
                            );

                        product["quantity"] =
                            sqlite3_column_int(
                                itemStmt,
                                3
                            );

                        order["products"].append(
                            product
                        );
                    }

                    sqlite3_finalize(itemStmt);
                }

                result["orders"].append(
                    order
                );
            }

            sqlite3_finalize(orderStmt);

            callback(
                HttpResponse::newHttpJsonResponse(result)
            );
        },
        {Get}
    );

    // =====================================================
    // SELLER ORDER HISTORY API
    // =====================================================

    app().registerHandler(
        "/api/orders/seller",
        [db](
            const HttpRequestPtr &req,
            std::function<void(
                const HttpResponsePtr &)> &&callback)
        {
            Json::Value result;

            result["success"] = true;
            result["orders"] =
                Json::arrayValue;

            std::string sellerIdText =
                req->getParameter("seller_id");

            if (sellerIdText.empty())
            {
                result["success"] = false;
                result["message"] =
                    "seller_id is required.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            int sellerId =
                std::stoi(sellerIdText);

            sqlite3_stmt *orderStmt = nullptr;

            const char *orderSQL = R"(
                SELECT DISTINCT
                    o.id,
                    o.customer_name,
                    o.mobile,
                    o.address,
                    o.city,
                    o.pincode,
                    o.payment,
                    o.total,
                    o.status,
                    o.order_date
                FROM orders o
                JOIN order_items oi
                    ON o.id = oi.order_id
                JOIN products p
                    ON oi.product_id = p.id
                WHERE p.seller_id = ?
                ORDER BY o.id DESC;
            )";

            if (
                sqlite3_prepare_v2(
                    db,
                    orderSQL,
                    -1,
                    &orderStmt,
                    nullptr
                ) != SQLITE_OK
            )
            {
                result["success"] = false;
                result["message"] =
                    "Failed to load seller orders.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            sqlite3_bind_int(
                orderStmt,
                1,
                sellerId
            );

            while (
                sqlite3_step(orderStmt)
                == SQLITE_ROW
            )
            {
                Json::Value order;

                int orderId =
                    sqlite3_column_int(
                        orderStmt,
                        0
                    );

                const char *customerName =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(
                            orderStmt,
                            1
                        )
                    );

                const char *mobile =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(
                            orderStmt,
                            2
                        )
                    );

                const char *address =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(
                            orderStmt,
                            3
                        )
                    );

                const char *city =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(
                            orderStmt,
                            4
                        )
                    );

                const char *pincode =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(
                            orderStmt,
                            5
                        )
                    );

                const char *payment =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(
                            orderStmt,
                            6
                        )
                    );

                double total =
                    sqlite3_column_double(
                        orderStmt,
                        7
                    );

                const char *status =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(
                            orderStmt,
                            8
                        )
                    );

                const char *orderDate =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(
                            orderStmt,
                            9
                        )
                    );

                order["id"] =
                    orderId;

                order["customer_name"] =
                    customerName
                        ? customerName
                        : "";

                order["mobile"] =
                    mobile
                        ? mobile
                        : "";

                order["address"] =
                    address
                        ? address
                        : "";

                order["city"] =
                    city
                        ? city
                        : "";

                order["pincode"] =
                    pincode
                        ? pincode
                        : "";

                order["payment"] =
                    payment
                        ? payment
                        : "";

                order["total"] =
                    total;

                order["status"] =
                    status
                        ? status
                        : "Placed";

                order["date"] =
                    orderDate
                        ? orderDate
                        : "";

                order["products"] =
                    Json::arrayValue;

                // GET ONLY THIS SELLER'S PRODUCTS

                sqlite3_stmt *itemStmt = nullptr;

                const char *itemSQL = R"(
                    SELECT
                        oi.product_id,
                        oi.product_name,
                        oi.price,
                        oi.quantity
                    FROM order_items oi
                    JOIN products p
                        ON oi.product_id = p.id
                    WHERE oi.order_id = ?
                    AND p.seller_id = ?;
                )";

                if (
                    sqlite3_prepare_v2(
                        db,
                        itemSQL,
                        -1,
                        &itemStmt,
                        nullptr
                    ) == SQLITE_OK
                )
                {
                    sqlite3_bind_int(
                        itemStmt,
                        1,
                        orderId
                    );

                    sqlite3_bind_int(
                        itemStmt,
                        2,
                        sellerId
                    );

                    while (
                        sqlite3_step(itemStmt)
                        == SQLITE_ROW
                    )
                    {
                        Json::Value product;

                        product["product_id"] =
                            sqlite3_column_int(
                                itemStmt,
                                0
                            );

                        const char *productName =
                            reinterpret_cast<const char *>(
                                sqlite3_column_text(
                                    itemStmt,
                                    1
                                )
                            );

                        product["name"] =
                            productName
                                ? productName
                                : "Product";

                        product["price"] =
                            sqlite3_column_double(
                                itemStmt,
                                2
                            );

                        product["quantity"] =
                            sqlite3_column_int(
                                itemStmt,
                                3
                            );

                        order["products"].append(
                            product
                        );
                    }

                    sqlite3_finalize(
                        itemStmt
                    );
                }

                result["orders"].append(
                    order
                );
            }

            sqlite3_finalize(
                orderStmt
            );

            callback(
                HttpResponse::newHttpJsonResponse(
                    result
                )
            );
        },
        {Get}
    );

    // =====================================================
    // ADD REVIEW API
    // =====================================================

    app().registerHandler(
        "/api/reviews",
        [db](
            const HttpRequestPtr &req,
            std::function<void(
                const HttpResponsePtr &)> &&callback)
        {
            auto json = req->getJsonObject();

            Json::Value result;

            if (!json)
            {
                result["success"] = false;
                result["message"] =
                    "Invalid JSON data.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            int productId =
                (*json)["product_id"].asInt();

            int buyerId =
                (*json)["buyer_id"].asInt();

            int rating =
                (*json)["rating"].asInt();

            std::string reviewText =
                (*json)["review_text"].asString();

            if (
                productId <= 0 ||
                buyerId <= 0 ||
                rating < 1 ||
                rating > 5 ||
                reviewText.empty()
            )
            {
                result["success"] = false;
                result["message"] =
                    "Invalid review data.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            sqlite3_stmt *stmt = nullptr;

            const char *sql = R"(
                INSERT INTO reviews
                (
                    product_id,
                    buyer_id,
                    rating,
                    review_text,
                    review_date
                )
                VALUES
                (
                    ?, ?, ?, ?,
                    datetime('now')
                );
            )";

            if (
                sqlite3_prepare_v2(
                    db,
                    sql,
                    -1,
                    &stmt,
                    nullptr
                ) != SQLITE_OK
            )
            {
                result["success"] = false;
                result["message"] =
                    "Database error.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            sqlite3_bind_int(
                stmt,
                1,
                productId
            );

            sqlite3_bind_int(
                stmt,
                2,
                buyerId
            );

            sqlite3_bind_int(
                stmt,
                3,
                rating
            );

            sqlite3_bind_text(
                stmt,
                4,
                reviewText.c_str(),
                -1,
                SQLITE_TRANSIENT
            );

            int stepResult =
                sqlite3_step(stmt);

            if (stepResult == SQLITE_DONE)
            {
                result["success"] = true;
                result["message"] =
                    "Review submitted successfully!";

                result["review_id"] =
                    static_cast<Json::Int64>(
                        sqlite3_last_insert_rowid(db)
                    );
            }
            else
            {
                result["success"] = false;
                result["message"] =
                    "Failed to submit review.";
            }

            sqlite3_finalize(stmt);

            callback(
                HttpResponse::newHttpJsonResponse(result)
            );
        },
        {Post}
    );

    // =====================================================
    // GET REVIEWS API
    // =====================================================

    app().registerHandler(
        "/api/reviews",
        [db](
            const HttpRequestPtr &req,
            std::function<void(
                const HttpResponsePtr &)> &&callback)
        {
            Json::Value result;

            result["success"] = true;
            result["reviews"] =
                Json::arrayValue;

            std::string productIdText =
                req->getParameter("product_id");

            if (productIdText.empty())
            {
                result["success"] = false;
                result["message"] =
                    "product_id is required.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            int productId =
                std::stoi(productIdText);

            sqlite3_stmt *stmt = nullptr;

            const char *sql = R"(
                SELECT
                    r.id,
                    r.rating,
                    r.review_text,
                    r.review_date,
                    u.name
                FROM reviews r
                LEFT JOIN users u
                    ON r.buyer_id = u.id
                WHERE r.product_id = ?
                ORDER BY r.id DESC;
            )";

            if (
                sqlite3_prepare_v2(
                    db,
                    sql,
                    -1,
                    &stmt,
                    nullptr
                ) != SQLITE_OK
            )
            {
                result["success"] = false;
                result["message"] =
                    "Failed to load reviews.";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            sqlite3_bind_int(
                stmt,
                1,
                productId
            );

            while (
                sqlite3_step(stmt)
                == SQLITE_ROW
            )
            {
                Json::Value review;

                review["id"] =
                    sqlite3_column_int(
                        stmt,
                        0
                    );

                review["rating"] =
                    sqlite3_column_int(
                        stmt,
                        1
                    );

                const char *reviewText =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(
                            stmt,
                            2
                        )
                    );

                review["review_text"] =
                    reviewText
                        ? reviewText
                        : "";

                const char *reviewDate =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(
                            stmt,
                            3
                        )
                    );

                review["date"] =
                    reviewDate
                        ? reviewDate
                        : "";

                const char *reviewerName =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(
                            stmt,
                            4
                        )
                    );

                review["reviewer"] =
                    reviewerName
                        ? reviewerName
                        : "Buyer";

                result["reviews"].append(
                    review
                );
            }

            sqlite3_finalize(stmt);

            callback(
                HttpResponse::newHttpJsonResponse(result)
            );
        },
        {Get}
    );

    // =====================================================
    // ADMIN USER MANAGEMENT
    // =====================================================

    app().registerHandler(
        "/api/admin/users",
        [db](
            const HttpRequestPtr &req,
            std::function<void(
                const HttpResponsePtr &)> &&callback)
        {
            Json::Value result;
            Json::Value users(Json::arrayValue);

            const char *sql =
                "SELECT id, name, email, role "
                "FROM users "
                "ORDER BY id DESC";

            sqlite3_stmt *stmt = nullptr;

            if (
                sqlite3_prepare_v2(
                    db,
                    sql,
                    -1,
                    &stmt,
                    nullptr
                ) != SQLITE_OK
            )
            {
                result["success"] = false;
                result["message"] =
                    "Failed to load users";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            while (
                sqlite3_step(stmt)
                == SQLITE_ROW
            )
            {
                Json::Value user;

                user["id"] =
                    static_cast<Json::Int64>(
                        sqlite3_column_int64(stmt, 0)
                    );

                const char *name =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(stmt, 1)
                    );

                const char *email =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(stmt, 2)
                    );

                const char *role =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(stmt, 3)
                    );

                user["name"] =
                    name ? name : "";

                user["email"] =
                    email ? email : "";

                user["role"] =
                    role ? role : "";

                users.append(user);
            }

            sqlite3_finalize(stmt);

            result["success"] = true;
            result["users"] = users;

            callback(
                HttpResponse::newHttpJsonResponse(result)
            );
        },
        {Get}
    );

    // =====================================================
    // ADMIN DELETE USER
    // =====================================================

    app().registerHandler(
        "/api/admin/users/delete",
        [db](
            const HttpRequestPtr &req,
            std::function<void(
                const HttpResponsePtr &)> &&callback)
        {
            Json::Value result;

            auto json =
                req->getJsonObject();

            if (
                !json ||
                !json->isMember("id")
            )
            {
                result["success"] = false;
                result["message"] =
                    "User ID required";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            long long id =
                (*json)["id"].asInt64();

            if (id <= 0)
            {
                result["success"] = false;
                result["message"] =
                    "Invalid user ID";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            const char *sql =
                "DELETE FROM users WHERE id = ?";

            sqlite3_stmt *stmt = nullptr;

            if (
                sqlite3_prepare_v2(
                    db,
                    sql,
                    -1,
                    &stmt,
                    nullptr
                ) != SQLITE_OK
            )
            {
                result["success"] = false;
                result["message"] =
                    "Failed to delete user";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            sqlite3_bind_int64(
                stmt,
                1,
                id
            );

            int stepResult =
                sqlite3_step(stmt);

            int changes =
                sqlite3_changes(db);

            sqlite3_finalize(stmt);

            if (
                stepResult != SQLITE_DONE
            )
            {
                result["success"] = false;
                result["message"] =
                    "Failed to delete user";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            if (changes == 0)
            {
                result["success"] = false;
                result["message"] =
                    "User not found";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            result["success"] = true;
            result["message"] =
                "User deleted successfully";

            callback(
                HttpResponse::newHttpJsonResponse(result)
            );
        },
        {Post}
    );

    // =====================================================
    // ADMIN ORDER MANAGEMENT
    // =====================================================

    app().registerHandler(
        "/api/admin/orders",
        [db](
            const HttpRequestPtr &req,
            std::function<void(
                const HttpResponsePtr &)> &&callback)
        {
            Json::Value result;
            Json::Value orders(Json::arrayValue);

            const char *sql = R"(
                SELECT
                    id,
                    customer_name,
                    mobile,
                    address,
                    city,
                    pincode,
                    payment,
                    total,
                    status,
                    order_date
                FROM orders
                ORDER BY id DESC
            )";

            sqlite3_stmt *stmt = nullptr;

            if (
                sqlite3_prepare_v2(
                    db,
                    sql,
                    -1,
                    &stmt,
                    nullptr
                ) != SQLITE_OK
            )
            {
                result["success"] = false;
                result["message"] =
                    "Failed to load orders";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            while (
                sqlite3_step(stmt)
                == SQLITE_ROW
            )
            {
                Json::Value order;

                long long orderId =
                    sqlite3_column_int64(
                        stmt,
                        0
                    );

                order["id"] =
                    static_cast<Json::Int64>(
                        orderId
                    );

                const char *customerName =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(stmt, 1)
                    );

                const char *mobile =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(stmt, 2)
                    );

                const char *address =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(stmt, 3)
                    );

                const char *city =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(stmt, 4)
                    );

                const char *pincode =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(stmt, 5)
                    );

                const char *payment =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(stmt, 6)
                    );

                double total =
                    sqlite3_column_double(stmt, 7);

                const char *status =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(stmt, 8)
                    );

                const char *date =
                    reinterpret_cast<const char *>(
                        sqlite3_column_text(stmt, 9)
                    );

                order["customerName"] =
                    customerName
                        ? customerName
                        : "";

                order["mobile"] =
                    mobile
                        ? mobile
                        : "";

                order["address"] =
                    address
                        ? address
                        : "";

                order["city"] =
                    city
                        ? city
                        : "";

                order["pincode"] =
                    pincode
                        ? pincode
                        : "";

                order["payment"] =
                    payment
                        ? payment
                        : "";

                order["total"] =
                    total;

                order["status"] =
                    status
                        ? status
                        : "Placed";

                order["date"] =
                    date
                        ? date
                        : "";

                // -------------------------------------------------
                // ORDER PRODUCTS
                // -------------------------------------------------

                Json::Value products(
                    Json::arrayValue
                );

                const char *itemSQL = R"(
                    SELECT
                        product_id,
                        product_name,
                        price,
                        quantity
                    FROM order_items
                    WHERE order_id = ?
                )";

                sqlite3_stmt *itemStmt =
                    nullptr;

                if (
                    sqlite3_prepare_v2(
                        db,
                        itemSQL,
                        -1,
                        &itemStmt,
                        nullptr
                    ) == SQLITE_OK
                )
                {
                    sqlite3_bind_int64(
                        itemStmt,
                        1,
                        orderId
                    );

                    while (
                        sqlite3_step(itemStmt)
                        == SQLITE_ROW
                    )
                    {
                        Json::Value product;

                        product["product_id"] =
                            static_cast<Json::Int64>(
                                sqlite3_column_int64(
                                    itemStmt,
                                    0
                                )
                            );

                        const char *productName =
                            reinterpret_cast<const char *>(
                                sqlite3_column_text(
                                    itemStmt,
                                    1
                                )
                            );

                        product["name"] =
                            productName
                                ? productName
                                : "Product";

                        product["price"] =
                            sqlite3_column_double(
                                itemStmt,
                                2
                            );

                        product["quantity"] =
                            sqlite3_column_int(
                                itemStmt,
                                3
                            );

                        products.append(
                            product
                        );
                    }

                    sqlite3_finalize(
                        itemStmt
                    );
                }

                order["products"] =
                    products;

                orders.append(
                    order
                );
            }

            sqlite3_finalize(stmt);

            result["success"] = true;
            result["orders"] = orders;

            callback(
                HttpResponse::newHttpJsonResponse(result)
            );
        },
        {Get}
    );

    // =====================================================
    // ADMIN UPDATE ORDER STATUS
    // =====================================================

    app().registerHandler(
        "/api/admin/orders/status",
        [db](
            const HttpRequestPtr &req,
            std::function<void(
                const HttpResponsePtr &)> &&callback)
        {
            Json::Value result;

            auto json =
                req->getJsonObject();

            if (
                !json ||
                !json->isMember("id") ||
                !json->isMember("status")
            )
            {
                result["success"] = false;
                result["message"] =
                    "Order ID and status required";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            long long id =
                (*json)["id"].asInt64();

            std::string status =
                (*json)["status"].asString();

            if (
                id <= 0 ||
                status.empty()
            )
            {
                result["success"] = false;
                result["message"] =
                    "Invalid order ID or status";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            const char *sql =
                "UPDATE orders "
                "SET status = ? "
                "WHERE id = ?";

            sqlite3_stmt *stmt = nullptr;

            if (
                sqlite3_prepare_v2(
                    db,
                    sql,
                    -1,
                    &stmt,
                    nullptr
                ) != SQLITE_OK
            )
            {
                result["success"] = false;
                result["message"] =
                    "Failed to update order";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            sqlite3_bind_text(
                stmt,
                1,
                status.c_str(),
                -1,
                SQLITE_TRANSIENT
            );

            sqlite3_bind_int64(
                stmt,
                2,
                id
            );

            int stepResult =
                sqlite3_step(stmt);

            int changes =
                sqlite3_changes(db);

            sqlite3_finalize(stmt);

            if (
                stepResult != SQLITE_DONE
            )
            {
                result["success"] = false;
                result["message"] =
                    "Failed to update order";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            if (changes == 0)
            {
                result["success"] = false;
                result["message"] =
                    "Order not found";

                callback(
                    HttpResponse::newHttpJsonResponse(result)
                );

                return;
            }

            result["success"] = true;
            result["message"] =
                "Order status updated successfully";

            callback(
                HttpResponse::newHttpJsonResponse(result)
            );
        },
        {Put}
    );

    // =====================================================
    // START SERVER
    // =====================================================

    std::cout
        << "Starting Vinoth Mart Backend...\n";

    // Render provides the PORT environment variable.
    // Local computer uses port 8080.

    int port = 8080;

    const char *portEnvironment =
        std::getenv("PORT");

    if (portEnvironment != nullptr)
    {
        try
        {
            port = std::stoi(portEnvironment);
        }
        catch (...)
        {
            std::cerr
                << "Invalid PORT environment variable. "
                << "Using port 8080.\n";

            port = 8080;
        }
    }

    std::cout
        << "Server starting on 0.0.0.0:"
        << port
        << "\n";

    app()
        .addListener(
            "0.0.0.0",
            port
        )
        .run();

    sqlite3_close(db);

    return 0;
}