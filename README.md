# TEE Arweave Deployment

```
    ├── App: The Application module in the TEE service, it uses json to communicate with the client.
    |
    ├── cmake: Compilation option script files related to Intel SGX SDK,
    |           please do not modify the files in this directory.
    |
    ├── Enclave: The trusted module in the TEE service, it runs in the Intel SGX trusted environment.
    |
    ├── 3rdparty
            |
            ├── mbedtls-sgx-2.6.0:  A trusted version of the mbedtls library, which provides 
            |                       cryptographic functions such as TLS, RSA, and HMAC.
            |
            ├── sgx-crypto-suites-tss-rsa:  An assembly of all the basic libraries and 
            |                               cryptography protocols from Safeheron
            |
            ├── tee-util-sgx-1.0.0: Trusted server base library, providing common functions for 
            |                       Intel SGX trusted service development.
```

## 1 Install SGX

### 1.1 Apply for Primary key

- Apply for **Primary key** and **Secondary key** at first, and them will be used later.
  The link address: [https://api.portal.trustedservices.intel.com/products](https://api.portal.trustedservices.intel.com/products).

- Click "**sign in**" to login, or click "**create a new IDZ account**" to create a new account.

- Click the link "**Intel® SGX Provisioning Certification Service**" in the page , and wait for the page to jump.

- Click the button "**Subscribe**" .

- Click the button "**Add subscription**" in the page and wait for the jump page.
  The **Primary key** and **Secondary key** are in the page, and you can use them.

### 1.2 Install DCAP

**Install dependencies**

```
$ sudo apt-get install dkms build-essential autoconf libtool pkg-config automake curl libgflags-dev libgtest-dev libc++-dev git vim zlib1g-dev openssl libssl-dev libboost-all-dev 
```

**Install sgx-driver**

- on ubuntu 18.04
  
  ```
  $ wget https://download.01.org/intel-sgx/sgx-linux/2.16/distro/ubuntu18.04-server/sgx_linux_x64_driver_1.41.bin
  $ chmod +x sgx_linux_x64_driver_1.41.bin
  $ ./sgx_linux_x64_driver_1.41.bin
  ```

- on ubuntu 20.04
  
  ```
  $ wget https://download.01.org/intel-sgx/sgx-linux/2.16/distro/ubuntu20.04-server/sgx_linux_x64_driver_1.41.bin
  $ chmod +x sgx_linux_x64_driver_1.41.bin
  $ ./sgx_linux_x64_driver_1.41.bin
  ```

**Install sgx-sdk**

- on ubuntu 18.04
  
  ```
  $ wget https://download.01.org/intel-sgx/sgx-linux/2.16/distro/ubuntu18.04-server/sgx_linux_x64_sdk_2.16.100.4.bin
  $ chmod +x sgx_linux_x64_sdk_2.16.100.4.bin
  $ ./sgx_linux_x64_sdk_2.16.100.4.bin   //Two questions will be asked, answer as follows
  ```
  
  Question: Do you want to install in current directory? [yes/no] :  **no**
  Question: Please input the directory which you want to install in :  **/opt/intel/**
  (if you install to the wrong directory, delete the sgxsdk, and install again)

- on ubuntu 20.04
  
  ```
  $ wget https://download.01.org/intel-sgx/sgx-linux/2.16/distro/ubuntu20.04-server/sgx_linux_x64_sdk_2.16.100.4.bin
  $ chmod +x sgx_linux_x64_sdk_2.16.100.4.bin
  $ ./sgx_linux_x64_sdk_2.16.100.4.bin   //Two questions will be asked, answer as follows
  ```
  
  Question: Do you want to install in current directory? [yes/no] :  **no**
  Question: Please input the directory which you want to install in :  **/opt/intel/**
  (if you install to the wrong directory, delete the sgxsdk, and install again)



**Install PCCS**

Fetch the Node.js setup script, because the PCCS package has a dependency on Node®.js version 14:

```
$ curl -o setup.sh -sL https://deb.nodesource.com/setup_14.x
$ chmod a+x setup.sh
$ sudo ./setup.sh
$ sudo apt-get -y install nodejs
```

We'll use Intel's Debian packages for the Intel SGX DCAP installation, but to do that we need to add the repository to the list of sources for apt:

- on ubuntu 18.04:
  
  ```
  $ sudo echo 'deb [arch=amd64] https://download.01.org/intel-sgx/sgx_repo/ubuntu bionic main' > /etc/apt/sources.list.d/intel-sgx.list
  $ wget -O - https://download.01.org/intel-sgx/sgx_repo/ubuntu/intel-sgx-deb.key | apt-key add -
  $ apt update
  ```

- on ubuntu 20.04:
  
  ```
  $ sudo echo 'deb [arch=amd64] https://download.01.org/intel-sgx/sgx_repo/ubuntu focal main' > /etc/apt/sources.list.d/intel-sgx.list
  $ wget -O - https://download.01.org/intel-sgx/sgx_repo/ubuntu/intel-sgx-deb.key | apt-key add -
  $ apt update
  ```

The PCCS will also need to compile a Node.js module for interfacing with C libraries, so we have to add the build-essential meta-package to the list:

```
$ sudo apt install sqlite3 python build-essential cracklib-runtime
$ sudo apt install sgx-dcap-pccs   //When installing this step, some questions will be asked, according to the following answers
```

> 1.Do you want to install PCCS now? (Y/N) :

**Answer: Y**

> 2.Enter your http proxy server address, e.g. http://proxy-server:port (Press ENTER if there is no proxy server):

**Press Enter**

> 3.Enter your https proxy server address, e.g. http://proxy-server:port (Press ENTER if there is no proxy server):

**Press Enter**

> 4.Do you want to configure PCCS now? (Y/N)

**Answer: Y**

> 5.Set HTTPS listening port [8081] (1024-65535)

**Accept default, Press Enter**

> 6.Set the PCCS service to accept local connections only? [Y] (Y/N)

**Answer: N**

> 7.Set your Intel PCS API key (Press ENTER to skip)

**Answer: (Enter your Primary key or Secondary key) for example: bb950d5eb195443f855b473e815db3f1**

> 8.Choose caching fill method : [LAZY] (LAZY/OFFLINE/REQ)

**Answer: REQ**

> 9.Set PCCS server administrator password:

**Answer: Set administrator password (Letters, characters and numbers) for example: DM2cEQOdSXuZblQW**    

> 10.Set PCCS server user password: 

**Answer: Set user password (Letters, characters and numbers) for example: SM76jxsGS7tc2BJ**
**Tt will be used later.** 

> 11.Do you want to generate insecure HTTPS key and cert for PCCS service? [Y] (Y/N):

**Answer: Y**

Then set the following certificate request. It is not important, because this is a self-signed certificate for testing and development purposes only. 

A production environment will require a certificate that is signed by a recognized certificate authority.

```
Country Name (2 letter code) [AU]:
State or Province Name (full name) [Some-State]:
Locality Name (eg, city) []:
Organization Name (eg, company) [Internet Widgits Pty Ltd]:
Organizational Unit Name (eg, section) []:
Common Name (e.g. server FQDN or YOUR name) []:
Email Address []:
A challenge password []:
An optional company name []:
```

Install the provisioning tools:
```
$ sudo apt install sgx-pck-id-retrieval-tool
$ sudo vi /opt/intel/sgx-pck-id-retrieval-tool/network_setting.conf 
```

- Uncomment the line "**PCCS_URL=https://localhost:8081/sgx/certification/v3/platforms**". (If you need, change the **PCCS_URL** to match your caching service's location.)
- Set "**USE_SECURE_CERT**" to "**FALSE**" and uncomment the line.
- Set the "**user_token**" parameter to the user password you created when configuring PCCS, and uncomment the line.
- Uncomment the line "**proxy_type = direct**". (If you need, set the **proxy_type** to fit your environment.)

Save your changes and run the provisioning tool:

```
$ PCKIDRetrievalTool
```

Install the Intel SGX runtime components, and Intel's reference quoting library and quote provider library.

```
$ sudo apt-get install libsgx-enclave-common-dev libsgx-dcap-ql-dev libsgx-dcap-default-qpl-dev
$ sudo vi /etc/aesmd.conf
```

Uncomment the line "**default quoting type=ecdsa_256**"

Restart AESM for the changes to take effect:

```
$ systemctl restart aesmd
$ sudo vi /etc/sgx_default_qcnl.conf
```

Set "**USE_SECURE_CERT**" to "**FALSE**" since we're using a self-signed certificate for testing purposes. Again, in a production environment, this should be set to "**TRUE**".

## 2 Install tools

**Install compiler**

Check the compiler version with the command **gcc --version** and **g++ --version**.  
If the compiler version is lower than 8.0, you need to install a higher version compiler, if it is higher than 8.0, ignore this step.

```
$ sudo apt install software-properties-common
$ sudo add-apt-repository ppa:ubuntu-toolchain-r/test
$ sudo apt install gcc-9 g++-9
$ sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 90 --slave /usr/bin/g++ g++ /usr/bin/g++-9
$ sudo update-alternatives --config gcc
```

If you have multiple versions of the compiler installed, switch between different versions of the compiler with the command:
**sudo update-alternatives --config gcc**



**Install cmake**

```
$ wget https://github.com/Kitware/CMake/releases/download/v3.22.3/cmake-3.22.3.tar.gz
$ tar -zxvf cmake-3.22.3.tar.gz
$ cd cmake-3.22.3
$ ./bootstrap --prefix=/usr/local/cmake
$ make
$ sudo make install

$ vim ~/.bash_aliases   //Add the following contents
  alias cmake=/usr/local/cmake/bin/cmake

$ source ~/.bash_aliases
```

**Install gtest**

```
$ git clone https://github.com/google/googletest
$ cd googletest
$ mkdir build && cd build
$ cmake ..
$ make
$ sudo make install
```

**Install protobuf**

```
$ git clone https://github.com/protocolbuffers/protobuf.git
$ cd protobuf
$ git checkout v3.14.0
$ git submodule update --init --recursive   //If this step fails, try multiple times until all downloads are complete
$ ./autogen.sh  
$ ./configure 
$ make
$ sudo make install
$ sudo cp /usr/local/lib/libprotobuf.so.25 /lib/x86_64-linux-gnu/
```

**Install cpprest**

```
$ git clone https://github.com/microsoft/cpprestsdk.git
$ cd cpprestsdk
$ git submodule update --init    //If this step fails, try multiple times until all downloads are complete
$ mkdir build && cd build
$ cmake ..
$ sudo make install
```

**Install crypto-suites-cpp**

```
$ git clone --recursive https://github.com/Safeheron/crypto-suites-cpp.git
$ cd crypto-suites-cpp
$ git submodule update --init --recursive    //If this step fails, try multiple times until all downloads are complete
$ mkdir build && cd build
$ cmake ..
$ sudo make install
```
