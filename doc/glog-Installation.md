# glog Installation

Install gflags first.

```shell
$ sudo apt-get install libgflags-dev
```

Then install glog.

```shell
$ git clone https://github.com/google/glog.git
$ cd glog
$ mkdir build && cd build
$ cmake .. 
$ make
$ sudo make install
```