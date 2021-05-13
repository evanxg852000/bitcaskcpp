# Bitcask++

https://github.com/rafaelkallis/adaptive-radix-tree



## How to build
```bash
mkdir build && cd build
conan install ..  -s build_type=Release --build
cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build .
./bin/app
```


## Registry
main: https://conan.io/center

secondary: https://bintray.com/conan/conan-center
nlohmann_json/3.7.3
cxxopts/2.2.0
spdlog/1.4.2


## How to run
Run using docker or docker-compose, also refer to `entrypoint.sh` for available script.
```bash
docker-compose up 
```

```bash
BUIDKIT=1 docker build -t cppimage .
docker run -it -v "$PWD":"/home/connan/project" cppimage bash
```

```bash
docker run -v "$PWD":"/home/connan/project" cppimage build
conan install ..  -s build_type=Release --build
cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build .
``` 


