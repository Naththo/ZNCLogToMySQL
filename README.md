# ZNCLogToMySQL

Logs data to MySQL. Data includes:

  - Channel messages

### How to compile
`env LIBS="-L/usr/lib -lmysqlcppconn" znc-buildmod LogToMySQL.cpp`

###Loading the module
`/msg *status loadmod LogToMySQL database:dbname;host:127.0.0.1;username:dbuser;password:thepassword`
