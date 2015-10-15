SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `element_info`;
CREATE TABLE IF NOT EXISTS `element_info` (
   `owner` varchar(25) NOT NULL DEFAULT ' ',
   `element` varchar(255) NOT NULL DEFAULT ' ',
   `pen` int NOT NULL DEFAULT '0',
   `res` int NOT NULL DEFAULT '0',
   `potency` int NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

ALTER TABLE `element_info` ADD UNIQUE `unique_index` ( `owner`, `element` );
