# BitcaskC++

BitcaskC++ is a c++ implementation of the bistcask storage format. You can find the
reference paper at [Bash Tehcnology](https://riak.com/assets/bitcask-intro.pdf). This is a high performance 
key/value store with a very high write throughput. it includes a library for embeding inside your application 
and a compatible server (wip). 

## Features

* Embeddedable library `bitcaskcpp`
* Builtin server (`bitcaskcpp-server`)
* Predictable read/write performance
* Low latency
* High throughput

## Is Bitcask right for me?

Bitcask is great for storing hundreds of thousands to millions of key/value pairs. It can be used when you need very high write throughput while maintaining predictable read throughput. I you are thinking of [LevelDb](https://github.com/google/leveldb), [RocksDB](http://rocksdb.org/) or any other key value store, then bitcask should 
be a great option to consider. It's important to note that bistcask keeps all its keys in memory. if your 
expected keyspace does not fit in RAM, Bitcask might not be the right storage engine for you. Note that this 
only concerns the keys not the values.


## Development

You can develop inside a docker container using GCC9 and connan. 

```bash
BUIDKIT=1 docker build -t cppimage .
docker run -it -v "$PWD":"/home/connan/project" cppimage bash
./entrypoint.sh install
./entrypoint.sh build
./entrypoint.sh test
```

```bash
BUIDKIT=1 docker build -t cppimage .
docker run -v "$PWD":"/home/connan/project" cppimage bash
conan install ..  -s build_type=Release --build
cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build .
``` 

## Depends on:

* C++ implementation of [adaptive radix tree](https://github.com/rafaelkallis/adaptive-radix-tree)
* C++ implementation [CRC32](https://github.com/d-bahr/CRCpp)