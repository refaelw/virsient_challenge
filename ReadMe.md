# Virscient Coding Challenge 
My solution to the coding challenge 
Name : Refael Whyte
Email : refael.whyte@gmail.com

## Build Instructions 
This is built on windows, linux support is coming in the next release. 

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
___________________________
| Magic Number | Filename |

Then the server checks to make sure the file does not exist, if not then the client sends the file buffer. 

The server receives the header, checks the disk for the files existence. sends back either send the file, or stop. 


 