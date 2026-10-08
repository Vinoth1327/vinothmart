INSERT INTO users (name, email, password, role)
SELECT 'Admin', 'admin@gmail.com', 'admin123', 'admin'
WHERE NOT EXISTS (SELECT 1 FROM users WHERE email = 'admin@gmail.com');

INSERT INTO users (name, email, password, role)
SELECT 'Demo Buyer', 'buyer@gmail.com', 'buyer123', 'buyer'
WHERE NOT EXISTS (SELECT 1 FROM users WHERE email = 'buyer@gmail.com');

INSERT INTO users (name, email, password, role)
SELECT 'Demo Seller', 'seller@gmail.com', 'seller123', 'seller'
WHERE NOT EXISTS (SELECT 1 FROM users WHERE email = 'seller@gmail.com');

-- Six built-in Vinoth Mart products are also stored in MySQL.
-- IDs 1-6 are fixed so they match the product IDs used by the single HTML page.
INSERT INTO products (id, seller_id, name, price, quantity, category, description, icon, is_default)
VALUES
(1, NULL, 'Rice', 60.00, 100, 'grocery', 'Premium quality rice', '🍚', TRUE),
(2, NULL, 'Milk', 50.00, 50, 'dairy', 'Fresh dairy milk', '🥛', TRUE),
(3, NULL, 'Apple', 120.00, 80, 'fruits', 'Fresh red apples', '🍎', TRUE),
(4, NULL, 'Bread', 40.00, 60, 'bakery', 'Fresh bakery bread', '🍞', TRUE),
(5, NULL, 'Eggs', 70.00, 120, 'food', 'Fresh farm eggs', '🥚', TRUE),
(6, NULL, 'Cooking Oil', 150.00, 75, 'grocery', 'Premium cooking oil', '🧴', TRUE)
ON DUPLICATE KEY UPDATE
    name=VALUES(name),
    price=VALUES(price),
    quantity=VALUES(quantity),
    category=VALUES(category),
    description=VALUES(description),
    icon=VALUES(icon),
    is_default=TRUE;
