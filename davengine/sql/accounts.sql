SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `accounts`;
CREATE TABLE IF NOT EXISTS `accounts` (
   `accountID` int NOT NULL DEFAULT '-1',
   `tagType` int NOT NULL DEFAULT '-1',
   `created_by` varchar(50) NOT NULL DEFAULT 'system',
   `created_on` varchar(35) NOT NULL DEFAULT '',
   `modified_by` varchar(50) NOT NULL DEFAULT 'system',
   `modified_on` varchar(35) NOT NULL DEFAULT '',
   `name` varchar(25) NOT NULL DEFAULT '',
   `password` varchar(30) NOT NULL DEFAULT '',
   `level` smallint NOT NULL DEFAULT '0',
   `pagewidth` smallint NOT NULL DEFAULT '80',
   `chatting_as` varchar(25) NOT NULL DEFAULT '',
   PRIMARY KEY (`accountID`),
   UNIQUE INDEX `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

INSERT INTO `accounts` VALUES ( '0', '0', 'Davenge', 'init_script', 'Davenge', 'init_script', 'Davenge', 'DaPxnNCnzgTfs', '5', '80', ' ' );
INSERT INTO `accounts` VALUES ( '1', '0', 'Davenge', 'init_script', 'Davenge', 'init_script', 'Pain',    'PavIfmXaIsKfQ', '5', '80', ' ' );

