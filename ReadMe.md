# Virscient Coding Challenge 
My solution to the coding challenge 
Name : Refael Whyte
Email : refael.whyte@gmail.com

## Build Instructions 

```
mkdir build 
cd build 
conan install --build=missing .. -s build_type=Release
cmake ..
cmake --build . --config Release
```

Windows is always a pain with linking between debug and release builds. 