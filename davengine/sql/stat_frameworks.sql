SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `stat_frameworks`;
CREATE TABLE IF NOT EXISTS `stat_frameworks` (
   `statFrameworkID` int NOT NULL DEFAULT '-1',
   `tagType` int NOT NULL DEFAULT '-1',
   `created_by` varchar(50) NOT NULL DEFAULT 'system',
   `created_on` varchar(35) NOT NULL DEFAULT '',
   `modified_by` varchar(50) NOT NULL DEFAULT 'system',
   `modified_on` varchar(35) NOT NULL DEFAULT '',
   `name` varchar(255) NOT NULL DEFAULT ' ',
   `softcap` int NOT NULL DEFAULT '0 ',
   `hardcap`  int NOT NULL DEFAULT '0',
   `softfloor`  int NOT NULL DEFAULT '0',
   `hardfloor`  int NOT NULL DEFAULT '0',
   `type` int NOT NULL DEFAULT '0',
   PRIMARY KEY (`statFrameworkID`),
   UNIQUE INDEX `name` ( `name` )
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
