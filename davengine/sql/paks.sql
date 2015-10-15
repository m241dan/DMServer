SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `paks`;
CREATE TABLE IF NOT EXISTS `paks` (
   `name` varchar(50) NOT NULL DEFAULT ' ',
   `type` smallint NOT NULL DEFAULT '-1',
   `label` varchar(255) NOT NULL DEFAULT ' ',
   `value` int NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

ALTER TABLE `paks` ADD UNIQUE `unique_index` ( `name`, `label` );
