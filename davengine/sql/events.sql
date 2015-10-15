SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `events`;
CREATE TABLE IF NOT EXISTS `events` (
   `argument` varchar(4098) NOT NULL DEFAULT 'null',
   `time` int NOT NULL DEFAULT '0',
   `owner` int NOT NULL DEFAULT '-1',
   `ownertype` int NOT NULL DEFAULT '0',
   `type` int NOT NULL DEFAULT '0',
   `lua_string` varchar(4098) NOT NULL DEFAULT 'null'   
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

