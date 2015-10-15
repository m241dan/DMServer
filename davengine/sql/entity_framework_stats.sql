SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `entity_framework_stats`;
CREATE TABLE IF NOT EXISTS `entity_framework_stats` (
   `entityFrameworkID` int NOT NULL DEFAULT '-1',
   `statFrameworkID` int NOT NULL DEFAULT '-1'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
