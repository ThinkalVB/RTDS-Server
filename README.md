# RTDS-Server
Resource Tracking Directory Service
## Getting Started

Active development in Visual Studio 2019. Repository will contain the VS solution for x64 C++ console application.
Boost Librarys are to be compiled and linked seperately.

### Prerequisites

* **Boost::asio**
* **Boost::bind**
* **Boost::date_time**

### Installing

RTDS is a console application.  
Make sure firewalls are set to allow traffic from the appliation.  
This RTDS server is designed for single directory use hence won't support REGISTER and SWITCH commands.  
Initially support for TCP on port 349 only.  
Port number, thread count and maximum connection limit can be set dynamically.  
Use #define PRINT_LOG for advanced log and #define PRINT_ERROR for errors only.  
For getting CLI iostrem output, use #define RTDS_CLI_MODE.  
Logs will be put into logs.txt - the name can be changed as per user need.  
Initial version only support IPv4, IPV6 can be targeted using #define RTDS_DUAL_STACK in RTDS.cpp during compilation.  
Feel free to modify and use it - and please do contribute to make it better.

### Coding Style 

1) Camel Casing for all the variables.
2) Data Structure names start with capital letter.
3) Suitable abbrevations are used as per necessity.
4) All private member functions andvariables should prefix _ "undescore" before name.

## Build & Testing

* [Packet Sender](https://packetsender.com/download/)
* [ISO C++ 17 Standard](https://en.cppreference.com/w/cpp/17/)
* [Visual Studio 2019](https://www.visualstudio.com/downloads/)

## Contributing

email@thinkalvb@gmail.com

## Versioning

- 1.0) Development Package. [Based on RTDS draft]

## Authors

* **Thinkal VB** - *Initial work* 

## License

This project is licensed under the MIT License - see the LICENSE.md file for details

## Briefing

RTDS is a resource tracking server for computers, services, or other resources connected to the Internet.
RTDS maintains a directory service which will keep track of the logical address of these resources on the internet.
RTDS is built to ensure the persistence and integrity of distributed services without compromising the simplicity of the process.
All RTDS commands and responses are strictly confined to ASCII character encoding.
The RTDS server follows a command/response model in a strictly sequential order.
RTDS supports mirroring which allows real time tracking of changes happening in a directory.
