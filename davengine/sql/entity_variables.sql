SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `entity_variables`;
CREATE TABLE IF NOT EXISTS `entity_variables` (
   `name` varchar(255) NOT NULL DEFAULT ' ',
   `value` varchar(255)  NOT NULL DEFAULT ' ',
   `owner` int NOT NULL DEFAULT '0',
   `type` tinyint NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
