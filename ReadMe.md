# Virscient Coding Challenge 
My solution to the coding challenge 
Name : Refael Whyte
Email : refael.whyte@gmail.com

## Build Instructions 
This is built on windows. No linux support. 

```
mkdir build 
cd build 
conan install --build=missing .. -s build_type=Release
cmake ..
cmake --build . --config Release
```

Windows is always a pain with linking between debug and release builds. 

## Architecture 
Its a classic client server relationship. Implemented a basic protocol on top to make the process quicker. 

Client Process
Sends header of information 
____________________________________________
| Magic Number | File Size | Filename Size |

Then the client sends 
____________
| Filename |

Then the client sends the file buffer. 

The server receives the header, checks the disk for the files existence. sends back either send the file, or stop. 

## Using 
The output is two programs 
    virsient_client 
    virsient_server 

Run the server and then use the client to transmit some files. 

## Known Issues
The server never exits cleanly. The accept() function is blocking so hangs on forever. 

## Future Updates 
 - Have the server check to see if file exists and checksums match before transmission
 - Implement checksum/SHA256 of buffer, currently always returns 0.  
 - Linux support. Windows socket is weird.  