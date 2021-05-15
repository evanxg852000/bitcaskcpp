# BitcaskC++
[![Language](https://img.shields.io/badge/language-C++-blue.svg)](https://isocpp.org/) [![Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization) [![License](https://img.shields.io/badge/license-Apache%202-blue.svg)](https://opensource.org/licenses/MIT)

[![Build Status](https://travis-ci.org/evanxg852000/bitcaskcpp.svg?branch=master)](https://travis-ci.org/evanxg852000/bitcaskcpp)

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
docker run -it -v "$PWD":"/home/connan/project" cppimage
conan install ..  -s build_type=Release --build
cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build .
``` 

## Depends on:

* [Adaptive Radix Tree](https://github.com/rafaelkallis/adaptive-radix-tree)
* [CRC32](https://github.com/google/crc32c)
* [Cxx-Opts](https://github.com/jarro2783/cxxopts/)
* [Crow](https://github.com/CrowCpp/crow)
* [Picobench](https://github.com/iboB/picobench)
* [Plog](https://github.com/SergiusTheBest/plog)

TODO:
- [ ] benchmark
- [ ] test concurency
- [ ] add crc & compaction trigger option 

```
===============================================================================
   Name (baseline is *)   |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
===============================================================================
           bistcask_put * |       8 |     0.030 |    3811 |      - |   262381.1
             bistcask_get |       8 |     0.043 |    5373 |  1.410 |   186085.5
           bistcask_put * |      64 |     0.178 |    2788 |      - |   358593.6
             bistcask_get |      64 |     0.323 |    5046 |  1.810 |   198147.9
           bistcask_put * |     512 |     1.404 |    2742 |      - |   364658.6
             bistcask_get |     512 |     2.299 |    4491 |  1.638 |   222661.0
           bistcask_put * |    4096 |     8.861 |    2163 |      - |   462255.8
             bistcask_get |    4096 |    13.229 |    3229 |  1.493 |   309629.5
           bistcask_put * |    8192 |    15.843 |    1934 |      - |   517059.5
             bistcask_get |    8192 |    24.587 |    3001 |  1.552 |   333177.9
===============================================================================
```


