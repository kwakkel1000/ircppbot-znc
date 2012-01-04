-- phpMyAdmin SQL Dump
-- version 3.4.5
-- http://www.phpmyadmin.net
--
-- Host: mysql.centravi.eu
-- Generation Time: Jan 04, 2012 at 11:28 PM
-- Server version: 5.1.49
-- PHP Version: 5.3.3-7+squeeze3

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Database: `centravi_znc9004`
--

-- --------------------------------------------------------

--
-- Table structure for table `ZncCommands`
--

CREATE TABLE IF NOT EXISTS `ZncCommands` (
  `id` int(2) NOT NULL AUTO_INCREMENT,
  `command` varchar(128) NOT NULL,
  `bind` varchar(128) NOT NULL,
  `access` int(5) NOT NULL DEFAULT '1001',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=20 ;

--
-- Dumping data for table `ZncCommands`
--

INSERT INTO `ZncCommands` (`id`, `command`, `bind`, `access`) VALUES
(1, 'stats', 'stats', 200),
(2, 'joinall', 'joinall', 800),
(3, 'voiceall', 'voiceall', 800),
(4, 'read', 'read', 800),
(5, 'simul', 'simul', 400),
(6, 'simulall', 'simulall', 800),
(7, 'search', 'search', 200),
(8, 'info', 'info', 200),
(9, 'adduser', 'adduser', 200),
(10, 'deluser', 'deluser', 200),
(11, 'adduser', 'add', 200),
(12, 'deluser', 'del', 200),
(13, 'resetpass', 'resetpass', 200),
(14, 'znccommands', 'commands', 1),
(15, 'znccommands', 'znccommands', 1),
(16, 'setbindhost', 'setbindhost', 800),
(17, 'broadcast', 'broadcast', 800),
(18, 'save', 'save', 200),
(19, 'sendstatus', 'sendstatus', 800);

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
