SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `account_characters`;
CREATE TABLE IF NOT EXISTS `account_characters` (
   `accountID` int NOT NULL DEFAULT '-1',
   `entityInstanceID` int NOT NULL DEFAULT '-1',
   `name` varchar(255) NOT NULL DEFAULT ' ',
   PRIMARY KEY (`entityInstanceId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

