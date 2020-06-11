# RTDS-Server
Resource Tracking Directory Server
## Getting Started

Active development in Visual Studio 2019. You can generate the project using CMake.
Boost Asio standalone is to be compiled and linked seperately.
Use VCPKG - package manager to manage dependencies.

### Prerequisites

* **Boost::asio**

### Installing

RTDS is a console application.  
Make sure firewalls are set to allow traffic from the appliation.  
Initially support for TCP on port 349 only.  
Port number, thread count and maximum connection limit can be set dynamically.  
Use #define PRINT_LOG to enable logging and #define PRINT_DEBUG_LOG for debug logs.  
Initial version only support IPv4, IPV6 can be targeted using #define RTDS_DUAL_STACK at compile time.  
Feel free to modify and use it - and please do contribute to make it better.  

### Coding style 

1) Camel Casing for all the variables.
2) Data Structure names start with capital letter.
3) Suitable abbrevations are used as per necessity.
4) All private member functions and variables should prefix _ "undescore" before name.

## Built And Test

* [MSVC Visual Studio 2019](https://www.visualstudio.com/downloads/) 
* [Oracle VM VirtualBox](https://www.oracle.com/virtualization/technologies/vm/virtualbox.html) 
* [Packet Sender](https://packetsender.com/download/) 
* [CMake](https://cmake.org/download/) 
* [GCC g++](https://gcc.gnu.org/) 


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