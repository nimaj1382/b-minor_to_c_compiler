This is a b-minor to c compiler which is written in c language.
Here's how to run the project:

### Running Cmake

First you need to go to the directory of the project.
Then you need to run the Cmake command like below:
```
cmake CMakeLists.txt
```

### Running Make

After that command, there will be several files added, one of which is Makefile. Then you need to run the make command:
```
make
```

### Run the program
Then we will have the executable file "b-minor_to_c_compiler", which takes a *.b file as an argument and creates a *.b.c file in the same directory which is the b-minor code compiled and translated to c. 
```
./b-minor_to_c_compiler example.b
```
And we'll have the example.b.c file in the directory which can be shown by this command:
```
cat example.b.c
```
