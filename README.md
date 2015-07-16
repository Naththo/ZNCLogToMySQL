# ZNCLogToMySQL

Logs data to MySQL. Data includes:

  - Channel messages

### How to compile
**Note:** If you are updating the module and are recompiling, make sure you unload the module with ZNC before you recompile. If you do not your ZNC will crash.

`env LIBS="-L/usr/lib -lmysqlcppconn" znc-buildmod LogToMySQL.cpp`

###Loading the module
`/msg *status loadmod LogToMySQL database:dbname;host:127.0.0.1;username:dbuser;password:thepassword`

###Table structure
    CREATE TABLE IF NOT EXISTS `chatlogs` (
      `id` int(11) NOT NULL AUTO_INCREMENT,
      `znc_user` varchar(50) DEFAULT NULL,
      `type` varchar(20) NOT NULL,
      `channel` varchar(50) NOT NULL,
      `identhost` varchar(70) DEFAULT NULL,
      `timestamp` varchar(12) DEFAULT NULL,
      `message` text,
      KEY `Index 1` (`id`)
    ) ENGINE=InnoDB DEFAULT CHARSET=latin1;
