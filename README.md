# Vinoth Mart - Complete Runnable Project

College capstone online shopping mart using:
- Java 17
- Spring Boot 3.5.6
- Maven
- MySQL
- HTML/CSS/JavaScript

## Features included

- Buyer login
- Seller login
- Admin login
- Buyer/Seller account registration
- Buyer product browsing and search/filter
- Product images
- Cart
- Checkout
- Order placement
- Buyer order history
- Seller add/edit/delete products
- Seller order management
- Admin product management
- Admin user management
- Admin order management and status update
- Buyer product reviews
- MySQL database initialization
- One front-end file: `src/main/resources/static/index.html`

## Default accounts

Admin:
- Email: `admin@gmail.com`
- Password: `admin123`

Buyer:
- Email: `buyer@gmail.com`
- Password: `buyer123`

Seller:
- Email: `seller@gmail.com`
- Password: `seller123`

## Run in VS Code

1. Install JDK 17.
2. Install Maven 3.6.3 or later.
3. Install MySQL Server and make sure the MySQL service is running.
4. Open `src/main/resources/application.properties`.
5. Set these values for your MySQL installation if needed:

`spring.datasource.username=...`

`spring.datasource.password=...`

6. Open the project folder in VS Code.
7. Open Terminal in the project root.
8. Run:

`mvn spring-boot:run`

9. Open:

`http://127.0.0.1:8080/`

The application automatically creates the `vinoth_mart` database if the MySQL account has permission, and Spring Boot creates the tables, six built-in products, and demo accounts.

## Build a JAR

Run:

`mvn clean package`

Then:

`java -jar target/vinoth-mart-1.0.0.jar`

## Important note about product images

The six built-in products use embedded SVG images, so no image folder is required for them.

Seller-added products can save an image URL/path in browser localStorage using the Product Image URL / Path field. The existing backend stores the seller product itself in MySQL; the image path is kept by the single-page front end so the project can stay as one HTML file.

## Security note

This is a college demonstration project. Passwords are stored as plain text to keep the database logic simple. Do not use this authentication design for a real production shopping website.
