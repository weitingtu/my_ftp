# A Multi-Client File Transfer Application
## Getting Started
### Prerequisites
```
gcc
```
### Installing
```
./make
```
## Usage
```
sparc1> ./myfypserver PORT_NUMBER
client> ./myftpclient <server ip addr> <server port> <list|get|put> <file>
```
### Example
Launch server
```
sparc1> ./myfypserver 12345
```
List files in the `./data/` repository directory of the server
```
client> ./myftpclient 127.0.0.1 12345 list
```
Download remote `./data/sample.dat` to local `./`
```
client> ./myftpclient 127.0.0.1 12345 get sample.dat
```
Upload local `./sample.dat` to remote `./data/`
```
client> ./myftpclient 127.0.0.1 12345 put sample.dat
```
