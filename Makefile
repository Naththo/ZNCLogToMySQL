ZNC_DEFAULT_MODULE_DIRECTORY = "$$HOME/.znc/modules"

LogToMySQL.so: LogToMySQL.cpp
	env LIBS="-L/usr/lib -lmysqlcppconn" znc-buildmod LogToMySQL.cpp
	@if [ -d $(ZNC_DEFAULT_MODULE_DIRECTORY) ]; then cp LogToMySQL.so $(ZNC_DEFAULT_MODULE_DIRECTORY)/LogToMySQL.so; else echo "Could not find the directory for ZNC modules, please move LogToMySQL.so manually."; fi;
