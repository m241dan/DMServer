SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `entity_stats`;
CREATE TABLE IF NOT EXISTS `entity_stats` (
   `statFrameworkID` int NOT NULL DEFAULT '-1',
   `owner` int NOT NULL DEFAULT '0',
   `perm_stat` int NOT NULL DEFAULT '0 ',
   `mod_stat`  int NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
