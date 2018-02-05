
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
Download remote file `./data/sample.dat` to local `./`
```
client> ./myftpclient 127.0.0.1 12345 get sample.dat
```
Upload local file `./sample.dat` to remote `./data/`
```
client> ./myftpclient 127.0.0.1 12345 put sample.dat
```
## Running the tests
### List files
Launch server
```
sparc1> ./myfypserver 12345
```
List files
```
client> ./myftpclient 127.0.0.1 12345 list
client> ascii.dat binary.dat ...
```
### Download files
Launch server
```
sparc1> md5sum ./data/ascii.dat
9e0448eb777f9103eb0641d16077fff9 ./data/ascii.dat
sparc1> md5sum ./data/binary.dat
ce2cd0d525e17a6bd370db688c06827f ./data/binary.dat
sparc1> ./myfypserver 12345
```
Download `ascii.dat`
```
client> ./myftpclient 127.0.0.1 12345 get ascii.dat
client> md5sum ./ascii.dat
9e0448eb777f9103eb0641d16077fff9  ./ascii.dat
```
Download `binary.dat`
```
client> ./myftpclient 127.0.0.1 12345 get binary.dat
client> md5sum ./binary.dat
ce2cd0d525e17a6bd370db688c06827f  ./binary.dat
```
Download non-existence file `sample.dat` 
```
client> ./myftpclient 127.0.0.1 12345 get sample.dat
file sample.dat doesn't exist
```
### Upload files
Launch server
```
sparc1> ./myfypserver 12345
```
Upload `ascii.dat`
```
client> md5sum ./ascii.dat
9e0448eb777f9103eb0641d16077fff9  ./ascii.dat
client> ./myftpclient 127.0.0.1 12345 put ascii.dat
```
Upload `binary.dat`
```
client> md5sum ./binary.dat
ce2cd0d525e17a6bd370db688c06827f  ./binary.dat
client> ./myftpclient 127.0.0.1 12345 put binary.dat
```
Upload non-existence file `sample.dat` 
```
client> ./myftpclient 127.0.0.1 12345 put sample.dat
file sample.dat doesn't exist
```
Check md5sum
```
sparc1> md5sum ./data/ascii.dat
9e0448eb777f9103eb0641d16077fff9 ./data/ascii.dat
sparc1> md5sum ./data/binary.dat
ce2cd0d525e17a6bd370db688c06827f ./data/binary.dat
```
