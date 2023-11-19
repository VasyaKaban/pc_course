CREATE DATABASE pc_db;
USE pc_db;

CREATE TABLE UserRank
(
	id INT AUTO_INCREMENT PRIMARY KEY NOT NULL,
	name VARCHAR(64)
);

INSERT INTO UserRank VALUES
(0, 'Admin'),
(0, 'Employee'),
(0, 'Accountant');

CREATE TABLE User
(
	login VARCHAR(64) PRIMARY KEY NOT NULL,
	sha256_pwd CHAR(64) NOT NULL,
	sname VARCHAR(64),
	name VARCHAR(64),
	pname VARCHAR(64),
	rank INT NOT NULL,

	FOREIGN KEY(rank) REFERENCES UserRank(id) ON DELETE CASCADE
);

CREATE TABLE Customer
(
	telephone BIGINT UNSIGNED PRIMARY KEY NOT NULL,
	sname VARCHAR(64),
	name VARCHAR(64),
	pname VARCHAR(64)
);

CREATE TABLE NewOrder
(
	id INT AUTO_INCREMENT PRIMARY KEY NOT NULL,
	customer_tel BIGINT UNSIGNED NOT NULL,
	date_of_start DATE NOT NULL,
	date_of_expire DATE NOT NULL,
	date_of_completion DATE DEFAULT NULL,

	FOREIGN KEY(customer_tel) REFERENCES Customer(telephone) ON DELETE CASCADE
);

CREATE TABLE Distributor
(
	id INT AUTO_INCREMENT PRIMARY KEY NOT NULL,
	name VARCHAR(64) NOT NULL
);

INSERT INTO Distributor VALUES
(0, 'AMD'),
(0, 'NVIDIA'),
(0, 'HP'),
(0, 'VIA'),
(0, 'ASUS'),
(0, 'Acer'),
(0, 'Lenovo'),
(0, 'DELL');

CREATE TABLE ItemType
(
	id INT AUTO_INCREMENT PRIMARY KEY NOT NULL,
	name VARCHAR(64) NOT NULL
);

INSERT INTO ItemType VALUES
(0, 'CPU'),
(0, 'GPU'),
(0, 'RAM'),
(0, 'Laptop'),
(0, 'Monitor'),
(0, 'Motherboard'),
(0, 'Keyboard'),
(0, 'Mice');

CREATE TABLE Item
(
	id INT AUTO_INCREMENT PRIMARY KEY NOT NULL,
	name VARCHAR(256) NOT NULL,
	cost DOUBLE NOT NULL,
	count_on_warehouse INT UNSIGNED NOT NULL,
	distributor_id INT NOT NULL,
	type_id INT NOT NULL,
	description TEXT NOT NULL,
	FOREIGN KEY(distributor_id) REFERENCES Distributor(id) ON DELETE CASCADE,
	FOREIGN KEY(type_id) REFERENCES ItemType(id) ON DELETE CASCADE
);

CREATE TABLE OrderedItem
(
	id INT AUTO_INCREMENT PRIMARY KEY NOT NULL,
	item_id INT NOT NULL,
	order_id INT NOT NULL,
	count_in_order INT UNSIGNED NOT NULL,
	FOREIGN KEY(item_id) REFERENCES Item(id) ON DELETE CASCADE,
	FOREIGN KEY(order_id) REFERENCES NewOrder(id) ON DELETE CASCADE
);

DELIMITER $$

CREATE PROCEDURE CheckUserExists(IN _login VARCHAR(64), IN _sha256_pwd CHAR(64), IN rank_name VARCHAR(64))
BEGIN
	SELECT EXISTS(SELECT User.login AS login, UserRank.name AS rank FROM User
	INNER JOIN UserRank ON User.rank = UserRank.id
	WHERE User.login = _login AND UserRank.name = rank_name AND User.sha256_pwd = _sha256_pwd);
END $$

CREATE PROCEDURE GetRanks()
BEGIN
	SELECT * FROM UserRank;
END $$

CREATE PROCEDURE GetUsers()
BEGIN
	SELECT User.login AS login, User.sname AS sname, User.name AS name, User.pname AS pname, UserRank.name AS rank
	FROM User
	INNER JOIN UserRank ON User.rank = UserRank.id;
END $$

CREATE PROCEDURE GetCustomers()
BEGIN
	SELECT Customer.telephone AS telephone, Customer.sname AS sname, Customer.name AS name, Customer.pname AS pname FROM Customer;
END $$

CREATE PROCEDURE GetOrders()
BEGIN
	SELECT NewOrder.id AS id, UNIX_TIMESTAMP(NewOrder.date_of_start) AS date_of_start, UNIX_TIMESTAMP(NewOrder.date_of_expire) AS date_of_expire, UNIX_TIMESTAMP(NewOrder.date_of_completion) AS date_of_completion,
		Customer.telephone AS c_telephone, Customer.sname AS c_sname, Customer.name AS c_name, Customer.pname AS c_pname
		FROM NewOrder
		INNER JOIN Customer ON NewOrder.customer_tel = Customer.telephone;
END $$

CREATE PROCEDURE GetOrderItems(IN new_order_id INT)
BEGIN
	SELECT OrderedItem.id AS id, OrderedItem.count_in_order AS count_in_order,
		Item.name AS name, Item.cost AS cost, Distributor.name AS distributor, Item.description AS description, ItemType.name AS type,
		(OrderedItem.count_in_order * Item.cost) AS common_cost
		FROM OrderedItem
		INNER JOIN Item ON OrderedItem.item_id = Item.id
		INNER JOIN Distributor ON Item.distributor_id = Distributor.id
		INNER JOIN ItemType ON Item.type_id = ItemType.id
		WHERE OrderedItem.order_id = new_order_id;
END $$

CREATE PROCEDURE GetItems()
BEGIN
	SELECT Item.id AS id, Item.name AS name, Item.cost AS cost, Item.count_on_warehouse AS count_on_warehouse, Distributor.name AS distributor, ItemType.name AS type, Item.description AS description
	FROM Item
	INNER JOIN Distributor ON Item.distributor_id = Distributor.id
	INNER JOIN ItemType ON Item.type_id = ItemType.id;
END $$

CREATE PROCEDURE GetDistributors()
BEGIN
	SELECT Distributor.id AS id, Distributor.name AS name FROM Distributor;
END $$

CREATE PROCEDURE GetItemTypes()
BEGIN
	SELECT ItemType.id AS id, ItemType.name AS name FROM ItemType;
END $$

CREATE PROCEDURE DeleteUser(IN _login VARCHAR(64))
BEGIN
	DELETE FROM User WHERE User.login = _login;
END $$

CREATE PROCEDURE CreateUser(IN _login VARCHAR(64), IN _sha256_pwd CHAR(64), IN _sname VARCHAR(64), IN _name VARCHAR(64), IN _pname VARCHAR(64), IN rank_id INT)
BEGIN
	IF EXISTS (SELECT * FROM User WHERE User.login = _login)
	THEN
		SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'UserAlreadyExists';
	ELSEIF NOT EXISTS (SELECT * FROM UserRank WHERE UserRank.id = rank_id)
	THEN
		SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'RankNotExists';
	ELSE
		INSERT INTO User VALUES(_login, _sha256_pwd, _sname, _name, _pname, rank_id);
	END IF;
END $$

CREATE PROCEDURE UpdateUser(IN _login VARCHAR(64), IN _sname VARCHAR(64), IN _name VARCHAR(64), IN _pname VARCHAR(64), IN rank_id INT)
BEGIN
	IF NOT EXISTS (SELECT * FROM UserRank WHERE UserRank.id = rank_id)
	THEN
		SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'RankNotExists';
	ELSE
		UPDATE User SET User.sname = _sname, User.name = _name, User.pname = _pname, User.rank = rank_id WHERE User.login = _login;
	END IF;
END $$

CREATE PROCEDURE CreateCustomer(IN _telephone BIGINT UNSIGNED, IN _sname VARCHAR(64), IN _name VARCHAR(64), IN _pname VARCHAR(64))
BEGIN
	IF EXISTS (SELECT * FROM Customer WHERE Customer.telephone = _telephone)
	THEN
		SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'CustomerAlreadyExists';
	ELSE
		INSERT INTO Customer VALUES(_telephone, _sname, _name, _pname);
	END IF;
END $$

CREATE PROCEDURE DeleteCustomer(IN _telephone BIGINT UNSIGNED)
BEGIN
	DELETE FROM Customer WHERE Customer.telephone = _telephone;
END $$

CREATE PROCEDURE AddItem(IN _name VARCHAR(256), IN _cost DOUBLE, IN _count INT UNSIGNED, IN _distributor_id INT, IN _type_id INT, IN _description TEXT)
BEGIN
	IF NOT EXISTS (SELECT * FROM Distributor WHERE Distributor.id = _distributor_id)
	THEN
		SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'DistributorNotExists';
	ELSE
		IF NOT EXISTS (SELECT * FROM ItemType WHERE ItemType.id = _type_id)
		THEN
			SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'ItemTypeNotExists';
		ELSE
			INSERT INTO Item VALUES(0, _name, _cost, _count, _distributor_id, _type_id, _description);
		END IF;
	END IF;
END $$

CREATE PROCEDURE AddItemCount(IN _id INT, IN appended INT)
BEGIN
	UPDATE Item SET Item.count_on_warehouse = Item.count_on_warehouse + appended WHERE Item.id = _id;
END $$

CREATE PROCEDURE AddOrderedItem(IN _order_id INT, IN _item_id INT, IN _count_in_order INT UNSIGNED)
BEGIN
	IF NOT EXISTS (SELECT * FROM Item WHERE Item.id = _item_id)
	THEN
		SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'ItemNotExists';
	ELSE
		INSERT INTO OrderedItem VALUES(0, _item_id, _order_id, _count_in_order);
	END IF;
END $$

CREATE PROCEDURE AddOrder(IN _customer_telephone BIGINT UNSIGNED, IN _date_of_expire DATE)
BEGIN
	IF NOT EXISTS (SELECT * FROM Customer WHERE Customer.telephone = _customer_telephone)
	THEN
		SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'CustomerNotExists';
	ELSE
		INSERT INTO NewOrder VALUES(0, _customer_telephone, CURRENT_TIMESTAMP(), _date_of_expire, NULL);
	END IF;
END $$

CREATE PROCEDURE CompleteOrder(IN _id INT)
BEGIN
	IF NOT EXISTS(SELECT * FROM NewOrder WHERE NewOrder.id = _id AND NewOrder.date_of_completion IS NOT NULL)
	THEN
		UPDATE Item,
		(SELECT Item.id AS id, SUM(OrderedItem.count_in_order) AS count_in_order
		FROM OrderedItem
		INNER JOIN Item ON OrderedItem.item_id = Item.id
		WHERE OrderedItem.order_id = _id
		GROUP BY Item.distributor_id) tbl
		SET Item.count_on_warehouse = Item.count_on_warehouse - tbl.count_in_order
		WHERE Item.id = tbl.id;

		UPDATE NewOrder SET date_of_completion = CURRENT_TIMESTAMP() WHERE NewOrder.id = _id;
	END IF;
END $$

CREATE TRIGGER complete_order_check BEFORE UPDATE ON Item
FOR EACH ROW
BEGIN
	IF NEW.count_on_warehouse < 0
	THEN
		SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = 'NotEnoughItems';
	END IF;
END $$

CREATE PROCEDURE GetExpiredOrders()
BEGIN
	SELECT NewOrder.id AS id, UNIX_TIMESTAMP(NewOrder.date_of_start) AS date_of_start, UNIX_TIMESTAMP(NewOrder.date_of_expire) AS date_of_expire, UNIX_TIMESTAMP(NewOrder.date_of_completion) AS date_of_completion, Customer.telephone AS c_telephone, Customer.sname AS c_sname, Customer.name AS c_name, Customer.pname AS c_pname
	FROM NewOrder
	INNER JOIN Customer ON Customer.telephone = NewOrder.customer_tel
	WHERE ((NewOrder.date_of_completion > NewOrder.date_of_expire) OR (NewOrder.date_of_completion IS NULL AND CURRENT_TIMESTAMP() > NewOrder.date_of_expire));
END $$

CREATE PROCEDURE GetCompletedOrders(IN _start DATE, IN _end DATE)
BEGIN
	SELECT NewOrder.id AS id, UNIX_TIMESTAMP(NewOrder.date_of_start) AS date_of_start, UNIX_TIMESTAMP(NewOrder.date_of_expire) AS date_of_expire, UNIX_TIMESTAMP(NewOrder.date_of_completion) AS date_of_completion, Customer.telephone AS c_telephone, Customer.sname AS c_sname, Customer.name AS c_name, Customer.pname AS c_pname
	FROM NewOrder
	INNER JOIN Customer ON Customer.telephone = NewOrder.customer_tel
	WHERE ((NewOrder.date_of_completion IS NOT NULL) AND (NewOrder.date_of_completion >= _start) AND (NewOrder.date_of_completion <= _end));
END $$

CREATE PROCEDURE GetIncome(IN _start DATE, IN _end DATE)
BEGIN
	SELECT COALESCE(SUM(OrderedItem.count_in_order * Item.cost), 0) AS income
		FROM OrderedItem
		INNER JOIN Item ON OrderedItem.item_id = Item.id
		INNER JOIN NewOrder ON OrderedItem.order_id = NewOrder.id
		WHERE ((NewOrder.date_of_completion IS NOT NULL) AND (NewOrder.date_of_completion >= _start) AND (NewOrder.date_of_completion <= _end));
END $$

CREATE PROCEDURE GetSellsStatiscticsByDistributors(IN _start DATE, IN _end DATE)
BEGIN
	SELECT SUM(Item.cost * OrderedItem.count_in_order) AS common_cost, SUM(OrderedItem.count_in_order) AS common_count, Distributor.name AS distributor
	FROM OrderedItem
	INNER JOIN Item ON Item.id = OrderedItem.item_id
	INNER JOIN Distributor ON Item.distributor_id = Distributor.id
	INNER JOIN NewOrder ON OrderedItem.order_id = NewOrder.id
	WHERE ((NewOrder.date_of_completion IS NOT NULL) AND (NewOrder.date_of_completion >= _start) AND (NewOrder.date_of_completion <= _end))
	GROUP BY Distributor.name;
END $$

DELIMITER ;

CALL CreateUser("amogus", "80c5e536eec8387cccad28b8b17b933832244998d85918abf18cc9bada5d4fe9", "amogus_sname", "amogus_name", "amogus_pname", 1);
CALL CreateUser("abobus", "32c9a2ec9e0c4e3a4cc93012b2e72c04b2c395578dcc80b535e951b452eaf9a3", "amogus_sname", "amogus_name", "amogus_pname", 2);
CALL CreateUser("amongaust", "a2c1492a15db8248ab62f9575d18753b5fda78b7ab3e89111af2ece23849a51c", "amogus_sname", "amogus_name", "amogus_pname", 3);

CALL CreateCustomer(123, "c1_name", "c1_sname", "c1_pname");
CALL CreateCustomer(456, "c2_name", "c2_sname", "c2_pname");
CALL CreateCustomer(789, "c3_name", "c3_sname", "c3_pname");

CALL AddItem("Ryzen 5 1600", 250.09, 189, 1, 1, "Ryzen 5 1600, 6 cores, 12 threads");
CALL AddItem("NVIDIA GTX 1080Ti", 1058.4, 53, 2, 2, "GPU NVIDIA GTX1080Ti");
CALL AddItem("Intel Core i3 9100F", 135.3, 456, 3, 1, "Intel Core i3 9100F 4 cores, 8 threads");

CALL AddOrder(123, CURRENT_TIMESTAMP() + INTERVAL 1 YEAR);

CALL AddOrderedItem(1, 1, 18);
CALL AddOrderedItem(1, 2, 7);
CALL AddOrderedItem(1, 3, 75);
CALL AddOrderedItem(1, 1, 9);
