# SGX-SSL Installation

The prerequisites and more details of sgxssl ,see [Intel SGX-SSL](https://github.com/intel/intel-sgx-ssl.git).

```shell
$ git clone https://github.com/intel/intel-sgx-ssl.git
$ cd intel-sgx-ssl/openssl_source
$ wget https://www.openssl.org/source/openssl-1.1.1q.tar.gz
$ cd ../Linux
$ make all test
$ sudo make install
```
