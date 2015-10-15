SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `entity_instances`;
CREATE TABLE IF NOT EXISTS `entity_instances` (
   `entityInstanceID` int NOT NULL DEFAULT '-1',
   `tagType` int NOT NULL DEFAULT '-1',
   `created_by` varchar(50) NOT NULL DEFAULT 'system',
   `created_on` varchar(35) NOT NULL DEFAULT '',
   `modified_by` varchar(50) NOT NULL DEFAULT 'system',
   `modified_on` varchar(35) NOT NULL DEFAULT '',
   `containedBy` int NOT NULL DEFAULT '-1',
   `frameworkID` int NOT NULL DEFAULT '-1',
   `live` int NOT NULL DEFAULT '0',
   `corpse_owner` int NOT NULL DEFAULT '0',
   `state` int NOT NULL DEFAULT '0',
   `mind` int NOT NULL DEFAULT '0',
   `tspeed` int NOT NULL DEFAULT '0',
   `isPlayer` int NOT NULL DEFAULT '0',
   `home` int NOT NULL DEFAULT '0',
   `height_mod` int NOT NULL DEFAULT '0',
   `weight_mod` int NOT NULL DEFAULT '0',
   `width_mod` int NOT NULL DEFAULT '0',
   `level` int NOT NULL DEFAULT '0',
   PRIMARY KEY (`entityInstanceID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
