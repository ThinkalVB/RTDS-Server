# RTDS-Server
Resource Tracking Directory Server
RTDS (UDP and TCP) Available at (IPv4 34.67.5.27 port 321)

## Getting Started

Active development in Visual Studio 2019. You can generate the project using CMake.
Boost Asio standalone and OpenSSL is to be compiled and linked seperately.
Use VCPKG - package manager to manage dependencies.

### Prerequisites

* **Boost::asio**
* **OpenSSL**

### Installing

RTDS is a console application.  
Make sure firewalls are set to allow traffic from the appliation (Run as root in Linux).  
Initially support for TCP and UDP on port 321 (default).  
Port number and thread count can be passed as arguments -p and -t (ex: rtds -p349 -t8).  
Use #define PRINT_LOG to enable logging and #define PRINT_DEBUG_LOG for debug logs.  
Use #define OUTPUT_DEBUG_LOG to print the logs to the console output stream.  
RTDS supports both IPv4 and IPv6[Not Tested]. IPv6 can be targeted using #define RTDS_DUAL_STACK at compile time.  

## Built And Test

* [MSVC Visual Studio 2019](https://www.visualstudio.com/downloads/) 
* [Oracle VM VirtualBox](https://www.oracle.com/virtualization/technologies/vm/virtualbox.html) 
* [Packet Sender](https://packetsender.com/download/) 
* [CMake](https://cmake.org/download/) 
* [GCC g++](https://gcc.gnu.org/) 


## Contributing

email@thinkalvb@gmail.com

## Versioning

- 1.0.0) Initial Beta Package. [Based on RTDS draft 1.0.0]
- 1.1.0) Beta Version. [Based on RTDS draft 1.1.0]

## Authors

* **Thinkal VB** - *Initial work* 

## License

This project is licensed under the MIT License - see the LICENSE.md file for details

## Briefing

RTDS is a resource tracking server for computers, services, or other resources connected to the Internet.
RTDS maintains a directory service which will keep track of the logical address of these resources on the internet.
RTDS is built to ensure the persistence and integrity of distributed services without compromising the simplicity of the process.
All RTDS commands and responses are strictly confined to ASCII character encoding.
The RTDS server follows a command-response model in a strictly sequential order.
Feel free to modify and use it - and please do contribute to make it better.