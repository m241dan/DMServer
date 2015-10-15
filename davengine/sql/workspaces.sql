SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `workspaces`;
CREATE TABLE IF NOT EXISTS `workspaces` (
   `workspaceID` int NOT NULL DEFAULT '-1',
   `tagType` int NOT NULL DEFAULT '-1',
   `created_by` varchar(50) NOT NULL DEFAULT 'system',
   `created_on` varchar(35) NOT NULL DEFAULT '',
   `modified_by` varchar(50) NOT NULL DEFAULT 'system',
   `modified_on` varchar(35) NOT NULL DEFAULT '',
   `name` varchar(20) NOT NULL DEFAULT 'unknown',
   `description` text NOT NULL DEFAULT '',
   `public` smallint NOT NULL DEFAULT '0',
   PRIMARY KEY (`workspaceID`),
   UNIQUE INDEX `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

