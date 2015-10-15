SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `entity_frameworks`;
CREATE TABLE IF NOT EXISTS `entity_frameworks` (
   `entityFrameworkID` int NOT NULL DEFAULT '-1',
   `tagType` int NOT NULL DEFAULT '-1',
   `created_by` varchar(50) NOT NULL DEFAULT 'system',
   `created_on` varchar(35) NOT NULL DEFAULT '',
   `modified_by` varchar(50) NOT NULL DEFAULT 'system',
   `modified_on` varchar(35) NOT NULL DEFAULT '',
   `name` varchar(255) NOT NULL DEFAULT '',
   `short_descr` varchar(255) NOT NULL DEFAULT '',
   `long_descr` varchar(255) NOT NULL DEFAULT '',
   `description` text NOT NULL DEFAULT '',
   `inheritsID` int NOT NULL DEFAULT '-1',
   `primary_dmg` int NOT NULL DEFAULT '-1',
   `tspeed` int NOT NULL DEFAULT '0',
   `spawn_time` int NOT NULL DEFAULT '0',
   `height` int NOT NULL DEFAULT '0',
   `weight` int NOT NULL DEFAULT '0',
   `width` int NOT NULL DEFAULT '0',
   PRIMARY KEY (`entityFrameworkID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

INSERT INTO `entity_frameworks` VALUES ( '0', '2', 'Davenge', 'initscript', 'Davenge', 'initscript', 'North', 'North', 'Northern exit is here.', 'An exit to the north.', '-1', '-1', '0', '0', '10', '200', '10' );
INSERT INTO `entity_frameworks` VALUES ( '1', '2', 'Davenge', 'initscript', 'Davenge', 'initscript', 'South', 'South', 'Southern exit is here.', 'An exit to the south.', '-1', '-1', '0', '0', '10', '200', '10' );
INSERT INTO `entity_frameworks` VALUES ( '2', '2', 'Davenge', 'initscript', 'Davenge', 'initscript', 'East', 'East', 'Easthern exit is here.', 'An exit to the east.', '-1', '-1', '0', '0', '10', '200', '10' );
INSERT INTO `entity_frameworks` VALUES ( '3', '2', 'Davenge', 'initscript', 'Davenge', 'initscript', 'West', 'West', 'Westhern exit is here.', 'An exit to the west.', '-1', '-1', '0', '0', '10', '200', '10' );
INSERT INTO `entity_frameworks` VALUES ( '4', '2', 'Davenge', 'initscript', 'Davenge', 'initscript', 'Up', 'Up', 'Up exit is here.', 'An exit leading up.', '-1', '-1', '0', '0', '10', '200', '10' );
INSERT INTO `entity_frameworks` VALUES ( '5', '2', 'Davenge', 'initscript', 'Davenge', 'initscript', 'Down', 'Down', 'Down exit is here.', 'An exit leaded down.', '-1', '-1', '0', '0', '10', '200', '10' );

