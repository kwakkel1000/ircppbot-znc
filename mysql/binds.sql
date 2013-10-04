SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";
-- --------------------------------------------------------

-- 
-- Table structure for table `binds`
-- 

CREATE TABLE IF NOT EXISTS `binds` (
  `command` varchar(128) NOT NULL,
  `alias` varchar(128) NOT NULL,
  `who` varchar(32) NOT NULL,
  `access` int(5) NOT NULL DEFAULT '-1',
  PRIMARY KEY (`alias`),
  UNIQUE KEY `alias` (`alias`,`who`) ) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- 
-- Dumping data for table `binds`
-- 

INSERT INTO `binds` (`command`, `alias`, `who`, `access`) VALUES
('version', 'versions', 'zncbot', 1000),
('listbinds', 'listbinds', 'zncbot', 1000);
