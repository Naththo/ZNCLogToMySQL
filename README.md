# ZNCLogToMySQL

Logs data to MySQL. Data includes:

  - Channel messages

### How to compile
`env LIBS="-L/usr/lib -lmysqlcppconn" znc-buildmod LogToMySQL.cpp`

###Loading the module
`/msg *status loadmod LogToMySQL database:dbname;host:127.0.0.1;username:dbuser;password:thepassword`

###Table structure
    CREATE TABLE IF NOT EXISTS `chatlogs` (
      `id` int(11) NOT NULL AUTO_INCREMENT,
      `znc_user` varchar(50) DEFAULT NULL,
      `type` enum('privmsg','chan','join') NOT NULL,
      `sender` varchar(50) NOT NULL,
      `identhost` varchar(70) DEFAULT NULL,
      `timestamp` varchar(50) DEFAULT NULL,
      `message` text,
      KEY `Index 1` (`id`)
    ) ENGINE=InnoDB DEFAULT CHARSET=latin1;

### TODO
- [x] Add kick logs
- [ ] Add quit logs
- [ ] Add nick change logs
- [ ] Add part logs
- [ ] Add notice logs
- [ ] Add mode logs
