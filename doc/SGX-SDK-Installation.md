# SGX-SDK Installation

Install dkms first.

```shell
$ sudo apt install dkms
```

- on ubuntu 18.04

```shell
$ wget https://download.01.org/intel-sgx/sgx-linux/2.17.1/distro/ubuntu18.04-server/sgx_linux_x64_sdk_2.17.101.1.bin
$ chmod +x sgx_linux_x64_sdk_2.17.101.1.bin
$ ./sgx_linux_x64_sdk_2.17.101.1.bin   //Two questions will be asked, answer as follows
```
Question: Do you want to install in current directory? [yes/no] :  **no**
Question: Please input the directory which you want to install in :  **/opt/intel/**
(if you install to the wrong directory, delete the sgxsdk, and install again)

- on ubuntu 20.04

```shell
$ wget https://download.01.org/intel-sgx/sgx-linux/2.17.1/distro/ubuntu20.04-server/sgx_linux_x64_sdk_2.17.101.1.bin
$ chmod +x sgx_linux_x64_sdk_2.17.101.1.bin
$ ./sgx_linux_x64_sdk_2.17.101.1.bin   //Two questions will be asked, answer as follows
```
Question: Do you want to install in current directory? [yes/no] :  **no**
Question: Please input the directory which you want to install in :  **/opt/intel/**
(if you install to the wrong directory, delete the sgxsdk, and install again)


**NOTE**: Due to SGX-SDK's protobuf missing util header directory，you must install protobuf v3.14 and copy the `xx/google/protobuf/util` directory to the `/opt/intel/sgxsdk/include/tprotobuf/google/protobuf/` directory to solve the missing header file issue.
