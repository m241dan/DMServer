SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `workspace_entries`;
CREATE TABLE IF NOT EXISTS `workspace_entries` (
   `workspaceID` int NOT NULL DEFAULT '-1',
   `entry` varchar(30) NOT NULL DEFAULT ' '
) ENGINE=InnoDB DEFAULT CHARSET=utf8;




