SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `live_specs`;
CREATE TABLE IF NOT EXISTS `live_specs` (
   `specType` varchar(255) NOT NULL DEFAULT ' ',
   `value` int NOT NULL DEFAULT '-1',
   `owner` varchar(255) NOT NULL DEFAULT ' '
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

INSERT INTO `live_specs` VALUES ( 'IsExit', '0', 'f0' );
INSERT INTO `live_specs` VALUES ( 'IsExit', '0', 'f1' );
INSERT INTO `live_specs` VALUES ( 'IsExit', '0', 'f2' );
INSERT INTO `live_specs` VALUES ( 'IsExit', '0', 'f3' );
INSERT INTO `live_specs` VALUES ( 'IsExit', '0', 'f4' );
INSERT INTO `live_specs` VALUES ( 'IsExit', '0', 'f5' );

INSERT INTO `live_specs` VALUES ( 'MirrorExit', '1', 'f0' );
INSERT INTO `live_specs` VALUES ( 'MirrorExit', '0', 'f1' );
INSERT INTO `live_specs` VALUES ( 'MirrorExit', '3', 'f2' );
INSERT INTO `live_specs` VALUES ( 'MirrorExit', '2', 'f3' );
INSERT INTO `live_specs` VALUES ( 'MirrorExit', '5', 'f4' );
INSERT INTO `live_specs` VALUES ( 'MirrorExit', '4', 'f5' );



