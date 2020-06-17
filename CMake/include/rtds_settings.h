#ifndef RTDS_SETTINGS_H
#define RTDS_SETTINGS_H

#define REGISTER_WARNING Error::_warnings++;
#define REGISTER_MEMMORY_ERR Error::_error_memmory++;
#define REGISTER_SOCKET_ERR Error::_error_socket++;
#define REGISTER_IO_ERR Error::_error_io++;
#define REGISTER_CODE_ERROR Error::_error_code++;

struct Error
{
	static int _warnings;
	static int _error_memmory;
	static int _error_socket;
	static int _error_io;
	static int _error_code;
};

#endif
