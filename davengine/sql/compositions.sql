SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `compositions`;
CREATE TABLE IF NOT EXISTS `compositions` (
   `owner` varchar(25) NOT NULL DEFAULT ' ',
   `element` varchar(255) NOT NULL DEFAULT ' ',
   `amount` int NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

ALTER TABLE `compositions` ADD UNIQUE `unique_index` ( `owner`, `element` );
