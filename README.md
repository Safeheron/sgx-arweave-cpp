# TEE arweave

## Introduction
![img](doc/logo.png)

本项目是基于Intel SGX开发的可信(TEE) ARWEAVE 服务，用于在可信坏境下生成RSA私钥分片,是为了解决在非可信坏境下生成RSA私钥分片时可能存在的数据泄露问题。

## Prerequisites
- [crypto-suits-cpp](https://github.com/safeheron/crypto-suites-cpp.git) 安全鹭的基本算法库，本项目用于hash、随机数、加解密等。
- [tss-rsa-cpp](https://github.com/Safeheron/tss-rsa-cpp.git) 安全鹭基于RSA的算法库，本项目用于生成RSA密钥分片。
- [sgx-crypto-suits-cpp] crypto-suits-cpp算法库的可信版本
- [sgx-tss-rsa-cpp] tss-rsa-cpp算法库的可信版本
- See the [Dependency-Installation](./doc/Dependency-Installation.md)，这是本项目的部署文档，必须先完成部署，才能运行本项目

## Build and Install

It only supports Linux now.
```shell
$ git clone https://github.com/Safeheron/sgx-arweave-cpp.git
$ cd sgx-arweave-cpp
$ mkdir build && cd build
$ make
$ make install
```
可执行文件和可信动态库在 tee-arweave-server-0.0.1 目录中。

## Usage

### configuration file
在运行之前，你必须先修改配置文件。
配置文件中的字段:
- **interface_create**: 创建私钥分片的接口，example: "/arweave/create_key_share"
- **interface_query**: 查询创建私钥分片进度的接口，example: "/arweave/query_key_shard_state";
- **call_back_address**: 回调地址，example: "http://127.0.0.1:8008/client";
- **log_path**: 日志路径，example: "/root/glog-arweave"
- **ip_address**: 服务端绑定的ip地址和端口，example: "http://0.0.0.0:40000"

for example:
```
```

### json 字段限制
post请求json结构体字段：

- **userPublicKeyList**：此字段，元素数量需与门限分母相等
- **k**：此字段为门限分子，分子不能大于分母；分子必须大于分母的一半
- **l**：此字段为门限分母，分母不能小于分子；分母不能小于2；分母不能超过20
- **keylength**: 此字段只支持1024，2048，3072，4096四种。

for example:
```json
{
	"userPublicKeyList": [  
        "049f992995affb335b576a7186316fc0ecfcca3d88f78dfb00e0e76e1f9a9766135230831442e4b1975f2caf81756a250032ea5e165ba1631606795be04a00d42c",   
        "04e30cd9f1283b95251e2721ee6f1fcbbc6ea56f32c924c0000f6f4e6a91d474dd1ff40d39fb8601b4b4066027952ede10e2d144f1b3aa5b2b1bf4210f4cc93e3d",   
        "0424c0853bdcb04fb8d50eaaa779f2c0d5f01c79b30b58f6a2fe739070e236cd142e32b8114f06b60b46b00f39745c874e8297ec9da01366927ac199072a103356"
	],
	"k": 2,
	"l": 3,
	"keyLength": 1024,
}
```

### 程序运行
修改完配置文件后，运行此项目。
使用 postman、APIpost 或者自己写个能发送 post 请求的程序。


### 报告验证
对于在可信环境下生成RSA私钥分片，需验证此可信环境是否真的可信。
所以我们需要对飞地生成的远程报告进行验证，我们提供了报告验证程序[sgx-arweave-client-js](https://github.com/Safeheron/sgx-arweave-client-js.git)
关于远程报告，可阅读intel[官方文档](https://download.01.org/intel-sgx/sgx-linux/2.17/docs/)



## Development Process & Contact

This library is maintained by Safeheron. Contributions are highly welcomed! Besides GitHub issues and PRs, feel free to reach out by mail.

