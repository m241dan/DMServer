SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `timers`;
CREATE TABLE IF NOT EXISTS `timers` (
   `owner` int NOT NULL DEFAULT '-1',
   `owner_type` int NOT NULL DEFAULT '-1',
   `timerkey` varchar(255) NOT  NULL DEFAULT ' ',
   `time` int NOT NULL DEFAULT '0',
   `frequency` int NOT NULL DEFAULT '0',
   `counter` int NOT NULL DEFAULT '0',
   `update_message` varchar(510) NOT NULL DEFAULT ' ',
   `end_message` varchar(51) NOT NULL DEFAULT ' ',
   `timer_type` int NOT NULL DEFAULT '-1',
   `active` int NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

