SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `entity_instance_possessions`;
CREATE TABLE IF NOT EXISTS `entity_instance_possessions` (
   `entityInstanceID` int NOT NULL DEFAULT '-1',
   `content_instanceID` int NOT NULL DEFAULT '-1'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

ALTER TABLE `entity_instance_possessions` ADD UNIQUE `unique_index` ( `entityInstanceID`, `content_instanceID` );
