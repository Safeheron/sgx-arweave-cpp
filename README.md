# Safeheron’s TEE Based RSA Key Shard Service

![img](doc/logo.png)

This project is an HTTP server of the TEE (Intel® SGX) based RSA key shard service to eliminate the security risks of generating RSA private key shards in a non-trusted centralized environment.




# Contents
* [Service Workflow](#service-workflow)
* [Deployment Document](#deployment-document)
* [Prerequisites](#prerequisites)
* [Build & Run](#build--run)
* [Usage](#usage)
    * [Configuration File](#configuration-file)
    * [Creation Request Details](#creation-request-details)
    * [Query Request Details](#query-request-details)
* [Intel® SGX Remote Attestation](#intel-sgx-remote-attestation)
* [Contributions & Contact](#contributions--contact)




# Service Workflow
This service provides two main functions: **Private Key Shard Creation** and **Creation Status Query**.
- **Private Key Shard Creation**: Create the private key shards and generate a remote attestation report. The request is processed asynchronously and will be returned immediately. When the request processing is finished, the service will send the generated key shards data and the certification report to the specified service address as an HTTP POST request by using a webhook.


- **Creation status query**: Query whether there is a matching task currently being created based on the request ID and return the query result.




# Deployment Document

You can follow the [Dependency-Installation](../../Desktop/8月13日/doc/Dependency-Installation.md) to deploy this service.




# Prerequisites

- [sgx-crypto-suits-cpp](https://github.com/Safeheron/sgx-crypto-suites-cpp): The SGX-supported version of **crypto-suits-cpp** library implemented by Safeheron.
- [sgx-tss-rsa-cpp](https://github.com/Safeheron/sgx-tss-rsa-cpp): The SGX-supported version of **tss-rsa-cpp** library implemented by Safeheron.




# Build & Run

It only supports Linux now.

```shell
$ git clone https://github.com/Safeheron/sgx-arweave-cpp.git
$ cd sgx-arweave-cpp
$ mkdir build && cd build
$ make
$ make install
$ cd tee-arweave-server-0.0.1
$ ./tee-arweave-server enclave.signed.so
```




# Usage

## Configuration File

Before you start, please modify the configuration file first.
The fields in the configuration file:

- `host_address` The host address of your server. example: `http://127.0.0.1:8008`;
- `key_shard_creation_path` The path of key shard creation request to your server, for example `/arweave/create_key_shard`;
- `key_shard_query_path`  The path of key shard query request to your server, for example `/arweave/query_key_shard`;
- `webhook_address` Webhook address, for example `http://127.0.0.1:8008/webhook`;
- `log_path` The absolute path to store logs, for example: `/root/glog-arweave`;




## Creation Request Details

### Interface URL Example
> http://YOUR_HOST/arweave/create_key_shard

### Request Method
> POST

### Content-Type
> JSON

### Request Body

Parameter |   Type   | Required  | Description
:---: |:--------:|:---------:| :---:
user_public_key_list |  String  |    YES    | This field represents the user public key list, which collects the ECDSA public key of each user. Users can decrypt the encrypted key shard information by using their private keys corresponding to the public keys.
k | Integer  |    YES    | Threshold numerator.
l | Integer  |    YES    | Threshold denominator.
key_length | Integer  |    YES    | The length of the private key to be generated. Only in 1024, 2048, 3072 and 4096.
webhook_url |  String  |    YES    | Webhook address. The generated results will be called back to this address via a webhook service.

```json
{
	"user_public_key_list": [
		"049f992995affb335b576a7186316fc0ecfcca3d88f78dfb00e0e76e1f9a9766135230831442e4b1975f2caf81756a250032ea5e165ba1631606795be04a00d42c",
		"04e30cd9f1283b95251e2721ee6f1fcbbc6ea56f32c924c0000f6f4e6a91d474dd1ff40d39fb8601b4b4066027952ede10e2d144f1b3aa5b2b1bf4210f4cc93e3d",
		"0424c0853bdcb04fb8d50eaaa779f2c0d5f01c79b30b58f6a2fe739070e236cd142e32b8114f06b60b46b00f39745c874e8297ec9da01366927ac199072a103356"
	],
	"k": 2,
	"l": 3,
	"key_length": 1024,
	"webhook_url": "http://127.0.0.1:8008/sayHi"
}
```

#### Example of the success response

```json
{
	"success": true,
	"message": "Request has been accepted."
}
```

#### Example of failure response

```json
{
	"success": false,
	"message": "Error Message"
}
```


### Parameter Rules

There are several parameter rules you should concern about:

- The number of the elements in `user_public_key_list` must equal the value of `l`;
- The value of `k` must be less than or equal the value of `l`;
- The value of `k` must be greater than the half of value of `l`;
- The value of `l` must be greater than or equal 2, less than or equal 20;


### Webhook Return Data

The generated key shards data and the certification report will be sent to the specified address defined in the **Configuration File** `webhook_address`.

#### Example of Webhook Return Data

Parameter | Type      | Description
--- |-----------| --- 
key_shard_pkg | JSON List | A JSON list to store the generated key shards data of each user.
pubkey_list_hash | String    | The hash of the concatenation of the public keys from `userPublicKeyList` generated by SHA-256 in hexadecimal.
tee_report | String    | The remote attestation report to prove that the service is currently running on Intel® SGX enabled platform. The report is encoded to Base64 format.

Parameter | Type   | Description
--- |--------| --- 
encrypt_key_info | String | A string encrypted by the public key of the user from `userPublicKeyList`. The plaintext is a serialized JSON object which contains each user's private key shard data, composed of `keyMeta` and `keyShard` information.
public_key | String | The ECDSA public key of the user from `userPublicKeyList` in hexadecimal.

```json
{
    "key_shard_pkg": [
        {
            "encrypt_key_info": "043048b5a9792819c8559293f16b8751ccd325072dfabfb2e895209710ecf0acc0da94b2df1cae5bbc5171f70ddc2b802d550be9e730968eee63567e9b76b15a7720e267c7e4533210ebd5ff6649d68b9c1544e4b3c1d0b2b18cd5fcd757954b9f4a6eb1969a17138dbd604853984f9816f499b821fd84c963e3d380b57e636eba2994fac9fdd8176ec1eefee58b51ceaacf54ae9f627bb7e65b11a0543350808619041f03324d7308a535a0f490ca5b17bd785fff7a2ef337d542342b0b69bfb44452548f12142448df0b52dbf412b2883e9960843412c0e390464bf0a29689831135fa95e3a88f29f75f5ec745a588faa7ab6665dd2c2c189a260089d966e140899d776add64c1e702a42221488612441acec5b3775a32aa936c5a691df148316558109934f41f0628154c37a3ed7b30aeb6fce36aee074b72cd271cc5648b11cf8f91dfb47b09d05add58eb55c76e2b413ad5bf1e3751495c50e0e368a6b4c2b5b95edaa444fe98c181066694f4624ace3bb4fc550b0a1e34b642ddcfa529b13e8bfbf29767c0180e752f7a02dda29289b61d110b492e44da6dad7f680838160879b12c403f489c6cb0c2cf3c213af3da2a36536c4e65fe5301abc1409ebabfd32dd6855ca91a7f5ae712519d336509d0d2c85b66df7d94c01bdba445c8b8931e3f80e56b01921e6f5afdc1c0d866e51e9b9ddf2142e0d8e242d3d14f226e3557079c542b82bb4e8d2a39ffae9fe35facdbf6bab6842cff6097192cf08bcedca6b7d8ce87d325cace7b7d14d04d9be022e4a924bd2b086dabde0408081ccc1c84daf09e626397d27e8c65219b496371fa35df48458b4ab697a6ee456534ddc5e42be75c0cc73f7c9a6872b6eafdd43eb6dcc585b65b391ac280c51955860319677d05d2956dce6c58c51ae47c39629888a6c185ab506cd41fd93b7d1349d29c7aa9d43a2ab65cbf9759a6ed32f9a64fa5da210990e2c3a91289c03426ae1c627c0bcb32dd47a2f4b6c66f49f6955e293241f5c43310fe7f4e2cc51fa50cd1bf157047395ff1b345f31cb9231093af03aa5bf4dba7c19c8c2813061265a582c7c8d799639880966fcfde525950d7c57a90d2c9791e1afab4e3f286ae52c0ab86d1b5423bc2db6a69f5b0d834e5905dbecb12b8d33807b5430482de95369007b22d3f15210cb7fede7d716a4b084bcabec38689098f5fa1c2734db070d30740890d5ccc1bf4caa836ad77a32080cb459edf0997e6cada381802fa078f3808cba1f31b99cba8f8580f433daaf55d3d87933138ea873ac602a959c3107ff4c3e040e62d151db4713eb998e64329cc5bebb49d56f36a30b39c1ef51231679010ade5aff461e4f564474a1b296696f2e0195f7349ca1ea4bd78358da05e08d2d86ef9bd6cca4228a43978f7a34975523d719c618991999ee8e7ebdb2bcff0c2517d4713e35152d3ebcb925edefb3f7ab3f6900081497ae29e1a8f7128daef775c1f1c9bdec8077b8366ffc2f0c192f0d0ded0ee136fcf900801e1ca29016a7ca02ec57bd8bdb70c1106f564f7e929a9c23eb8c69336f18fdcce7d2208f7be5662fef27c2c1a71d351f9a35a8d7210cc2dd3167ecca71840a8a5411d7a686221142362c92406bbcabdc6a4d991ac7faf6eda3f5f9ee3afedd2d48c82e2c6c7f55eb439f6f83b471f35f76bbdc461480095f1918db8dd945998ca10b1250c7adc4922b303e43bc6f62b89f5f6224608d72f905d62ff819c0dd06e2088eef63b47035d37ef499367f9e2ea6189cc252e1bfc8268427282f1c73b2f172cc3730795e645a53ab1232c3502d2bef152c9141d84655e2e271ad9140537e2e4a7e696006c1ed9ffafe9e200ac336224dbd14f269429cf341d51a43fa8b09c71693d59be089733d860fbbedb930aa362fff72357fed420422552f34cde3b2eb73462dd614a3a178214bd5b0c11b6811c3d1443723d89484a623d178327e9d93294456391ef73587daf5e48ff0a682f455b8dffccdef3f7c2a8606821b283b7b18e46caff9f94ed923943d45c6d90dc3834b324982a050e4a0ed6e2132a2816193f5cb6da9230e51cf6650167ccdcff094031132271774be564c7fe2331f0760c4e0e1817bcde4eaa6bd8813e5cc1af42d6b2734290e0044692a601bc5f5f99d6503bc8c798ffb957be981cb6fa6f01840eb8015b1af828b4b610e8139f0738d73129539c274861c1d1173e23b8c67131c188ec5f4274a94443cfc75d2d75a55b584710f759fa08fa817e434ed7e80ee368d3915c34396b198fc0b9a773907f93df7ff857df01966c76217caffc85603eebf14960fc60503e340614913440de84a504d0781ed7a42004fbaaae02864740d2843364c3f43e85adba5086f08f271b3c1fb42e5074080fc79a4e8d5d334ef4161405ba6a630b144aaafb1e4037cdd240ca73fc356fcad110e5892a9704520a53d371472683bce19aeb1684492257062e7cf9e78e8437a8a5cdee70bc493651301add15cd29f2aaeac6561924a919861350c5b9a4ba36f81be979d5a0e38fae699af21b348f8a1707ec9b77403fae7c8a6880e4301d54f87a7de26c8a45b880e042e1ad9527b9",
            "public_key": "0424c0853bdcb04fb8d50eaaa779f2c0d5f01c79b30b58f6a2fe739070e236cd142e32b8114f06b60b46b00f39745c874e8297ec9da01366927ac199072a103356"
        },
        {
            "encrypt_key_info": "04dc20b1cdb19f45be36ea5e62e420371e400823ff68290c6c5a623f9ee2a1bbb36589dd9a2ed23148c6a1394ae9f1f8a8d9f3b64a08ff28af32659e4d8ee7abe3b5173fd9a6d46d2963110751245b9c72a636278f1d571b0b078e7c84ec1b6574267339d25086fc3f85f4d1f7bf39c03d51a358cfa1dfed90a0eb62b8708a4cf77f48cfcf2df0fc6d9ab4efb19c76279e1d80c6925ec1d80590ac4286fc351a893ab2a2d76e6a6ce2d1234bc47e3e8d8941086da7d8a9465ffc8f9e09115d15ca513c93feeb3fda23da88589d26efbc7951f81080622950ba4d2c1e61fa813023283de581b9fc2d47da3dd487b913224afc54b3ac2fec575fad332028d9c5e36297b1b959c072309a65f6dbeb9c7ae0b63086fb8292be143ad2732d78d4be1a36fce3be57674b319d6b28748cf2aa1798bacf31df08c13d2ebac7919614ee7e0364344cfbc579b76a0e80834a3569f98eea6adbb57cc771a72535969420767b416a1f7a25169880617a39122a14ea742c6d5623294a830f645d6efd74def99c04398626f7c5dc2dde7b3b628c425e0562f7eb5b041fd0b8afb60203cdb895a7bb36caf4882cdc998a5cfed0c9b449c7ef84145774fba3a2f14739a614ee42ebf808fdf65a3071068ed69ff0406cef08e6fcc7b573f16e5ed292bf588c1d5c26003cd48bdf93b87496b52f635dc86ea6dba3a191d640efa42f18694343a65f18945ad80152e13d6f3c562bcd95f866349610b69a267a7d495e2876ae7d8854f20ecc0801643b72d0770482fa1014990a7f492c513250d02053d41b4942256a148ce4286c5d424eaa2f1ca3aa37691639eb5b4a969e30a1f1bf285e2543b187b4a4153d70d3422bc18965648525f514b374b7032665b9f841fd84fc5a75d28880ba71e27c657fe73ec7abebcee16c7a661803901cd19f68c640471eb4d95718decdabbdc19c5eae22d7492dea4947da78ee2ea1ece86fbaafcc271184e86f39a546492dd2147221109c97d00b89f7b993d5ca715ac7b07f536b407d7a806feb0fd2ab86a90ea572a693a90322459ec938a99a43f855ed285a69c849a48e314be429682a4b113aa8f64694e9fbe088dff2f0d4a5c3ce3dfd58329d8b43ef4fdbb7a621468078e3c68f0a97b8e243731523ff68dfba758511d90c922e17b8793bb6b031476b0f9bbeae480962b755941a9f4b6d2b0c10aafc2215d36a4718c4740e1719af6d60c6429c61ce84dc53f5a056e0459172b614d6522989e28e48105c0fdc9ed41a00c0bac5b13a47f364ac11303a863f6a08488ee14cfce42824216e32556213856002c1ffcb635b6015a4c8135953317e5a81dc4cac5f7616e76c778f90226a649b072b580911a9205a1d611477af7c0fafa5ac52f9fd444ad89d8f48c99c2bce208e860828530ed608000e2a0db93d793a0d6d85277b14b1a0f72fa52c0e3fd23c87a2c3b39d36916db224ac956fc84fcafed32ff5c0490356ed36dedef1091976ffe0d73504302de9b18efb24241f627a52735633307f50f43ef08057f32af64dd0b05f07e6af12719e2ab016ca0624e4a9579414bba581c86f276691f4e1ee06f7b54464fccf7b3c9e560eec475fd43407fbc7e109d2fc3aa7d69d21f2549ffca032845cec7dbd05f71f542320e03f1f9cf44c24729eeee1fae08961f4ec0cb398c0f942da804e47be58b148486ae8bc518f01d0b92c186ce630f33a54b6d07552249522f2de13513a3c7e38360066dc464b4abfb62d3c7cfb50d5ef3735f94fee3e1c05625e3e863903a73c2222f28b21fd09726db33946e337a9a7a3046afeb8c159f486b9ea64057738ddb64985f9653f8162737cc8e731a8b0cc9c56ab7b59b28d26e739831ed40289b8b9f8b43ab130c7902eed3a4b669d45273dfd180e6e27c53f1e20782286f2d525072302492fce9e01dbe6a9ff274a0dcc4165e253e640ebbf79f699d8e8eeccf612480c3a0e8414a2f8f7c48cf6ad0952b2d1881014bd6e5d6fa3b154bfc39ee049d598be1f050b10e5c64e5e86c5f96ad268d8489982c120e1bbd43516b8d7ee3bd45c27c6cb38d2341f8295011f039bcf3281e5c290cdc556c064f81ad03086c3d70f5bee5f6c03c3171ba2490d4e671cde4179b7cdbdc482f16df8865cd8c343b02c66471c6772c67fd3b187eaaf93ad753401ba371d0463bb5d8e5efe0648264560bdaa70f181a8f2e77852f0c6229ca7f2176772f6bda29e22d611e77a62d1c908f679c1b2a374b601bf57538fd3c767a56b4de5736998d6bfab9aca8c5db6adef64527fa035659c2809439e8548c374ff3f0ee3ddcb72726154e3f90624d50f268e92aa086f903808b3d7232b69ed86945586d5af0b72376b79555ab62bfa4ed250fec561b4540cd39e2e77c84bf8adc219e84c229ffbecd8fe0f4d603239cc4ff8de6d7899361e5edea6b90f9dda3be7076ec697a767d3ce8fb1904ebe933a3feb10dbd71726b4883a9b1c055ed860580bfc0d992ae53560463b6acba7012d09fb91351c51c48e8f725ecdced30d69bc775cc728740a48e3cd21e6cf9bf96fd38afaba098fca9783537d44f9618aa22518a52ba32fd0a2a48246f65278",
            "public_key": "049f992995affb335b576a7186316fc0ecfcca3d88f78dfb00e0e76e1f9a9766135230831442e4b1975f2caf81756a250032ea5e165ba1631606795be04a00d42c"
        },
        {
            "encrypt_key_info": "040497eb6411f411f85695515c71ad0b0fdd27218ede667f8ff489a7b5bd4816ccf28ee7afd81171c70ffcea584dc88a0f6ca8617e4f23da3895b133d8a9ccfdbbe9156040b1e01d03cb8f4d066a91a336ac511d8ffa8bbc1cc1bc0413dabf8349f1b505121cf126a10dad3e4645e9e121c9be7f6efb6df1915c2be3a3acc33712001ab2f373ed6238a5ea32d229020f29edcb224ce9995d4a0c4b07048420adcfc1b9d9cff7d56a7a712802e58a3c6823875a1a77c01382065c53b15204d7260393c3f6d3ec9dfb6cbe03f3bca4c858d21cebdfed42352ca13eee17b43e203a7cf12e4d25551988e95893da036579b218d783732e7e0994940dee9341c07495c4b91c1bca421db5fe3021cd0beb0f357643f32f53b8d0e34fd53be1d0f2c1eaef9ed6eec9f90f608616313971d6d2861e239e0e0fc9b71151ae4fab26534f2c39b0b3f347c726c0638c6b380c50b3e4d301af82a0d37ec4029447cb1dd27b8562de8b7f54c3fe97987d8490fc876414f618333ca4cc56a76bd762736c60027b25f622baddc194b5ecd33b0c01e6eaec877bf605e488c26eaed0847c80584949bc603b2b6f60234d03d97e868718e98d236b3f1f84b8a9f252bc199634a85556a496967f215b27136e54189dcfe75d9c8014838d618406a2e97e1f53887fd7711a2992b952676be66d5fb6e4c70d476b552fdf3c6b074e7fd57eb2f9efd4894162e6978c1f06dd910dae330396532ac8ac4382bb03912d2ae44f397f1d920cb4ed93f1296421058ed867ee7fd9eea455a25af41ed31250a1ebed3527192f249e9cfd858309f9865077d21332c62354093a9da65289d3b2582cb5023a6d2e1ca647f44da4f3ff6b409e1cd6031acb5df086ca0923c3b52ccfe3fd4db78cb3f11de30ab2297b3aeb31674d1a408d7e56ab64e9031eb9ee123805de1ba9d8e4d65f1361f0ee7770fc8f358bd19e2284d2be0303df76dc0a1fa4b9ef4d49bc8bba649148727064f3f8a22ee8e2d0702b6d0e31cd0762c6b57298c0bae21a7bd96ae9596e1f60a4f874b383b96362582283e6997b67556bf9b5c66d3be7fdec458fdb3df72eec6ec79e7ed7f6c2995bf79cab534b449a107dbbdffd18a6e4c8fc3c02a8a9117448002c5487ef3b377ffb499189b34b53a670ff960d02dfbe9baab9a4c7ba78cc5c4cdcc397e349bb73c80a9c295e24187f2bdb809cf7cd78289c26de716cb7d170548995324904d6dadbe9857f74929a3e460a339b57b2ca527faab4d1aba04a8b686bb97f7cbf437d8464f9325fe754517177b602f25a04e8e6380ffb8d5769deae0f2242abeb428e96678fd80f7af360906726e5922df40738b300b138fb93e8590414a3cfbe061fa0db9a86c29edfbd699220db72c59ed27aca1e6861f64e33c36676dc9512c9a3ea651b04b3a1a05bcc8757d00f5f6b257d30264dbfce1baf9e9fdd718fb15ca8cf2845149b4df34019973d645c1f78f5fd812cca303a9f275dbf8cbb4a828f424e91d1459c49ddbbe6416490011b4d5c5d7ecb04ac41a5a5aee0ffc5c09661f43c23b5c373f77a5fa49f5d9e3a788d8f238999207cb47c500cb9dc55b85908de6c83a40452440bc51e47f9ce7da2b1f74daf09ee65576e1b0f03f82dce596785d4aa921589b73abfcc72c60994d5b34b8ab1a4922538cff0fc51bba6bcfa3af138370e1e386439630296819c9ef72fcea09f26eb4859feeb6fe3dc4e30054510be29808b5f241a02a5c33249631fd130096cc5a61f5cb6b1b4123b7a2a5e9f77e3d302bb7b29806b6b8617056f3517a6159fbeefd120bfa43dd407b29057cbdc1f2e8e9cfef3b8d63c244047146ec91d0f516fdf008fb849ce7f35e9f64bc35b634f1a32658726efc5cc69c92f46d180bcd038a43bab54fd60eb9afd45747df7f4766d7c9722aed7e1fcf2c07e8d8b65ae19528e8faed3a6ebc81f2122facd92e42df6a4a059af881c8a229158cd3062e7093975323f53972e3bd3561700f764b9194325a7b7ec584d95721d96e8b29fdb5d4136472e3c60a7ba738d34c7f04e305470e14b531056a4adf1424df5e877b63b82e3822081bdb49322495068bc78087dd7995181927c9a200592a132d89f7f74a39465c4a7853498b43b1fb2a18d5a7531dd434d234828201e03507a914a2c4578684f14f4e62afce5d4648d8a7330b177cd3f50ec230e10326d15cace70afeae55f210947ce7ecd8864c53ac1d1d04be99b5f8d392aecc15b5959d2bfe4804f459a36a8b93cff9057b9a88ca655b878bb7fd47168168d198df4d3de52e7bb8398b39707e1450c34d42197223464724ba4364093d1dffd9209485f753c0ef5f920a39f6b61db6f182c4ca07ec67f3bef97cdba84e2b7340d8fee7cbcb7eb9f63d5e557861e390447372246b85ce7d58ba956c36c24694790aa116bb38c51390e0d131db51e1efc375c92b8c19f8a5c96ebeec5cc15106d37c5131f0504ba2ac8ff374322c3d07e541b3adcda96bcb890e427c97ca7a52acefd8dd2d4d7a5faff658cceeeca5e395ab5d4de2783cdc786da05c61a3f6e6ab0795c434443e92c4cf7c31b755984be6907448f8253958ce6b69b",
            "public_key": "04e30cd9f1283b95251e2721ee6f1fcbbc6ea56f32c924c0000f6f4e6a91d474dd1ff40d39fb8601b4b4066027952ede10e2d144f1b3aa5b2b1bf4210f4cc93e3d"
        }
    ],
    "pubkey_list_hash": "f6fca699eae08f95900e06744487f421a9e516c896efb72af4a7aad375c30636",
    "tee_report": "AwACAAAAAAAIAA0Ak5pyM/ecTKmUCg2zlX8GBy2X2CVjiLXgeJBLN9tt8MkAAAAABQUICf//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABQAAAAAAAADnAAAAAAAAAFRUb6WepiozzI3oqubMpA9SVpqWe4x8bdGT5LBnO/QdAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACD1xnnferKFHD2uvYqTXdDA8iZ22kCD5xw7h38CMfOngAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACD2MUxdzPCwC/X9B3C8VPkxCrCHVYEvO+1dXP83nucaQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAyRAAAAQMxomTa6yBzi65H7LeEOI37Il62FrmeodBTFoLQqTFRAc2icrvVbd4cbr/tkNiXbd8379BG+0qByN+w9p4BF1WCTJNIRm3aaNV/6qNb3UAUROcpUCJAi3X2ThBPgpekDoPjFZLiXT3rw/3e71LRM2plquj0fEDeT9D/Uwa3wNZBQUICf//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAFQAAAAAAAADnAAAAAAAAAIzlhoW+NuRhh8Izx+me1v5127M/dWetohewd+zYz4L5AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACMT1d115ZQPpYTf3fGioKaAFasje1wFAsIGwlEkMV7/wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAEACAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAviobr5ifeZ36aGOOvp3ILx2nE22vO0BfITMPO8L5lpQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAUBWMmb8Iv0VjjZ4OD1HtIhJs9bqQPsoYFTs/97PYKMbzs8O8mbloxLJqf1smUa2MjI6f4gzf6k4fHBOADkepXCAAAAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8FAGEOAAAtLS0tLUJFR0lOIENFUlRJRklDQVRFLS0tLS0KTUlJRTh6Q0NCSm1nQXdJQkFnSVZBSnoraFR4ZzV3OFNGU0tkY2xRU3djMWg0SnBiTUFvR0NDcUdTTTQ5QkFNQwpNSEF4SWpBZ0JnTlZCQU1NR1VsdWRHVnNJRk5IV0NCUVEwc2dVR3hoZEdadmNtMGdRMEV4R2pBWUJnTlZCQW9NCkVVbHVkR1ZzSUVOdmNuQnZjbUYwYVc5dU1SUXdFZ1lEVlFRSERBdFRZVzUwWVNCRGJHRnlZVEVMTUFrR0ExVUUKQ0F3Q1EwRXhDekFKQmdOVkJBWVRBbFZUTUI0WERUSXlNRGd3TmpFMU5UWXlPRm9YRFRJNU1EZ3dOakUxTlRZeQpPRm93Y0RFaU1DQUdBMVVFQXd3WlNXNTBaV3dnVTBkWUlGQkRTeUJEWlhKMGFXWnBZMkYwWlRFYU1CZ0dBMVVFCkNnd1JTVzUwWld3Z1EyOXljRzl5WVhScGIyNHhGREFTQmdOVkJBY01DMU5oYm5SaElFTnNZWEpoTVFzd0NRWUQKVlFRSURBSkRRVEVMTUFrR0ExVUVCaE1DVlZNd1dUQVRCZ2NxaGtqT1BRSUJCZ2dxaGtqT1BRTUJCd05DQUFSSgo3U3AvYmllL3IzNndJdXloTkNIYjh5bVl0OHZ5NllIdGJFc3NIOUl0bGZZbE5SS3Qza3QyNk10NnVnZnc3S2lVClBWeDVTTzJXTGw3MEJMVGk3Q0JQbzRJRERqQ0NBd293SHdZRFZSMGpCQmd3Rm9BVWxXOWR6YjBiNGVsQVNjblUKOURQT0FWY0wzbFF3YXdZRFZSMGZCR1F3WWpCZ29GNmdYSVphYUhSMGNITTZMeTloY0drdWRISjFjM1JsWkhObApjblpwWTJWekxtbHVkR1ZzTG1OdmJTOXpaM2d2WTJWeWRHbG1hV05oZEdsdmJpOTJNeTl3WTJ0amNtdy9ZMkU5CmNHeGhkR1p2Y20wbVpXNWpiMlJwYm1jOVpHVnlNQjBHQTFVZERnUVdCQlE2L1N5VFF0QkRVVHV0NjdRc2NMTFQKNmpJRC96QU9CZ05WSFE4QkFmOEVCQU1DQnNBd0RBWURWUjBUQVFIL0JBSXdBRENDQWpzR0NTcUdTSWI0VFFFTgpBUVNDQWl3d2dnSW9NQjRHQ2lxR1NJYjRUUUVOQVFFRUVPZ1paN3dqVmJheW9pcjJqU0VjbzFFd2dnRmxCZ29xCmhraUcrRTBCRFFFQ01JSUJWVEFRQmdzcWhraUcrRTBCRFFFQ0FRSUJCREFRQmdzcWhraUcrRTBCRFFFQ0FnSUIKQkRBUUJnc3Foa2lHK0UwQkRRRUNBd0lCQXpBUUJnc3Foa2lHK0UwQkRRRUNCQUlCQXpBUkJnc3Foa2lHK0UwQgpEUUVDQlFJQ0FQOHdFUVlMS29aSWh2aE5BUTBCQWdZQ0FnRC9NQkFHQ3lxR1NJYjRUUUVOQVFJSEFnRUFNQkFHCkN5cUdTSWI0VFFFTkFRSUlBZ0VBTUJBR0N5cUdTSWI0VFFFTkFRSUpBZ0VBTUJBR0N5cUdTSWI0VFFFTkFRSUsKQWdFQU1CQUdDeXFHU0liNFRRRU5BUUlMQWdFQU1CQUdDeXFHU0liNFRRRU5BUUlNQWdFQU1CQUdDeXFHU0liNApUUUVOQVFJTkFnRUFNQkFHQ3lxR1NJYjRUUUVOQVFJT0FnRUFNQkFHQ3lxR1NJYjRUUUVOQVFJUEFnRUFNQkFHCkN5cUdTSWI0VFFFTkFRSVFBZ0VBTUJBR0N5cUdTSWI0VFFFTkFRSVJBZ0VMTUI4R0N5cUdTSWI0VFFFTkFRSVMKQkJBRUJBTUQvLzhBQUFBQUFBQUFBQUFBTUJBR0NpcUdTSWI0VFFFTkFRTUVBZ0FBTUJRR0NpcUdTSWI0VFFFTgpBUVFFQmdCZ2FnQUFBREFQQmdvcWhraUcrRTBCRFFFRkNnRUJNQjRHQ2lxR1NJYjRUUUVOQVFZRUVCbTlBSmpiCjNack9VTlVkclZQYWFtVXdSQVlLS29aSWh2aE5BUTBCQnpBMk1CQUdDeXFHU0liNFRRRU5BUWNCQVFIL01CQUcKQ3lxR1NJYjRUUUVOQVFjQ0FRSC9NQkFHQ3lxR1NJYjRUUUVOQVFjREFRSC9NQW9HQ0NxR1NNNDlCQU1DQTBnQQpNRVVDSVFEakhyZ0Z1eEZ5ZlFpMC9taXYvN0hISzkvNEdHVkNFVnQzNUJDSWNaSXlud0lnWlI5a3RHUmppcUdpCjlMdEdNb1VpRXJxWHk4S2pCL3VQRXBrUng0TWtUdkE9Ci0tLS0tRU5EIENFUlRJRklDQVRFLS0tLS0KLS0tLS1CRUdJTiBDRVJUSUZJQ0FURS0tLS0tCk1JSUNsakNDQWoyZ0F3SUJBZ0lWQUpWdlhjMjlHK0hwUUVuSjFQUXp6Z0ZYQzk1VU1Bb0dDQ3FHU000OUJBTUMKTUdneEdqQVlCZ05WQkFNTUVVbHVkR1ZzSUZOSFdDQlNiMjkwSUVOQk1Sb3dHQVlEVlFRS0RCRkpiblJsYkNCRApiM0p3YjNKaGRHbHZiakVVTUJJR0ExVUVCd3dMVTJGdWRHRWdRMnhoY21FeEN6QUpCZ05WQkFnTUFrTkJNUXN3CkNRWURWUVFHRXdKVlV6QWVGdzB4T0RBMU1qRXhNRFV3TVRCYUZ3MHpNekExTWpFeE1EVXdNVEJhTUhBeElqQWcKQmdOVkJBTU1HVWx1ZEdWc0lGTkhXQ0JRUTBzZ1VHeGhkR1p2Y20wZ1EwRXhHakFZQmdOVkJBb01FVWx1ZEdWcwpJRU52Y25CdmNtRjBhVzl1TVJRd0VnWURWUVFIREF0VFlXNTBZU0JEYkdGeVlURUxNQWtHQTFVRUNBd0NRMEV4CkN6QUpCZ05WQkFZVEFsVlRNRmt3RXdZSEtvWkl6ajBDQVFZSUtvWkl6ajBEQVFjRFFnQUVOU0IvN3QyMWxYU08KMkN1enB4dzc0ZUpCNzJFeURHZ1c1clhDdHgydFZUTHE2aEtrNnorVWlSWkNucVI3cHNPdmdxRmVTeGxtVGxKbAplVG1pMldZejNxT0J1ekNCdURBZkJnTlZIU01FR0RBV2dCUWlaUXpXV3AwMGlmT0R0SlZTdjFBYk9TY0dyREJTCkJnTlZIUjhFU3pCSk1FZWdSYUJEaGtGb2RIUndjem92TDJObGNuUnBabWxqWVhSbGN5NTBjblZ6ZEdWa2MyVnkKZG1salpYTXVhVzUwWld3dVkyOXRMMGx1ZEdWc1UwZFlVbTl2ZEVOQkxtUmxjakFkQmdOVkhRNEVGZ1FVbFc5ZAp6YjBiNGVsQVNjblU5RFBPQVZjTDNsUXdEZ1lEVlIwUEFRSC9CQVFEQWdFR01CSUdBMVVkRXdFQi93UUlNQVlCCkFmOENBUUF3Q2dZSUtvWkl6ajBFQXdJRFJ3QXdSQUlnWHNWa2kwdytpNlZZR1czVUYvMjJ1YVhlMFlKRGoxVWUKbkErVGpEMWFpNWNDSUNZYjFTQW1ENXhrZlRWcHZvNFVveWlTWXhyRFdMbVVSNENJOU5LeWZQTisKLS0tLS1FTkQgQ0VSVElGSUNBVEUtLS0tLQotLS0tLUJFR0lOIENFUlRJRklDQVRFLS0tLS0KTUlJQ2p6Q0NBalNnQXdJQkFnSVVJbVVNMWxxZE5JbnpnN1NWVXI5UUd6a25CcXd3Q2dZSUtvWkl6ajBFQXdJdwphREVhTUJnR0ExVUVBd3dSU1c1MFpXd2dVMGRZSUZKdmIzUWdRMEV4R2pBWUJnTlZCQW9NRVVsdWRHVnNJRU52CmNuQnZjbUYwYVc5dU1SUXdFZ1lEVlFRSERBdFRZVzUwWVNCRGJHRnlZVEVMTUFrR0ExVUVDQXdDUTBFeEN6QUoKQmdOVkJBWVRBbFZUTUI0WERURTRNRFV5TVRFd05EVXhNRm9YRFRRNU1USXpNVEl6TlRrMU9Wb3dhREVhTUJnRwpBMVVFQXd3UlNXNTBaV3dnVTBkWUlGSnZiM1FnUTBFeEdqQVlCZ05WQkFvTUVVbHVkR1ZzSUVOdmNuQnZjbUYwCmFXOXVNUlF3RWdZRFZRUUhEQXRUWVc1MFlTQkRiR0Z5WVRFTE1Ba0dBMVVFQ0F3Q1EwRXhDekFKQmdOVkJBWVQKQWxWVE1Ga3dFd1lIS29aSXpqMENBUVlJS29aSXpqMERBUWNEUWdBRUM2bkV3TURJWVpPai9pUFdzQ3phRUtpNwoxT2lPU0xSRmhXR2pibkJWSmZWbmtZNHUzSWprRFlZTDBNeE80bXFzeVlqbEJhbFRWWXhGUDJzSkJLNXpsS09CCnV6Q0J1REFmQmdOVkhTTUVHREFXZ0JRaVpReldXcDAwaWZPRHRKVlN2MUFiT1NjR3JEQlNCZ05WSFI4RVN6QkoKTUVlZ1JhQkRoa0ZvZEhSd2N6b3ZMMk5sY25ScFptbGpZWFJsY3k1MGNuVnpkR1ZrYzJWeWRtbGpaWE11YVc1MApaV3d1WTI5dEwwbHVkR1ZzVTBkWVVtOXZkRU5CTG1SbGNqQWRCZ05WSFE0RUZnUVVJbVVNMWxxZE5JbnpnN1NWClVyOVFHemtuQnF3d0RnWURWUjBQQVFIL0JBUURBZ0VHTUJJR0ExVWRFd0VCL3dRSU1BWUJBZjhDQVFFd0NnWUkKS29aSXpqMEVBd0lEU1FBd1JnSWhBT1cvNVFrUitTOUNpU0RjTm9vd0x1UFJMc1dHZi9ZaTdHU1g5NEJnd1R3ZwpBaUVBNEowbHJIb01zK1hvNW8vc1g2TzlRV3hIUkF2WlVHT2RSUTdjdnFSWGFxST0KLS0tLS1FTkQgQ0VSVElGSUNBVEUtLS0tLQo="
}
```

NOTE: A **User Data** segment in the report is an identifier of 1-to-1 relationship between the report and the request during the verification of the report.

The formula of the **User Data** is:

![img](../../Desktop/8月13日/doc/formula.png)

#### Example of Plaintext JSON Object 
```json
{
	"key_meta": {
		"k": 2,
		"l": 3,
		"vkv": "92880508AEAAF864A28FB18B905822E56FCB345858301A6C00A4958C3AFD0DF19845B47AE48557E5D97DD77784B9E45FD18FBFDF6C1ED337FFA0F0C7001E793E2A56958A456B3DF3C2C94C9FF4C64DF85B17B34B73F4103BB5D4C58A1506B36E2C935341237DC6F75C02D282D90ADC8B59BECDDCDB49CB67EE0638E3FE21E6F9",
		"vku": "3AA66E665E7C92D92C482CB5B572317D371D69CF512B3E90421390638FE98D7ECF278F4D99C4387AAAEFE85E008FA53D844FF8DA9EB88B1098843A949D1AFAE32F5AB2B473F64B9F93B47E1C69B1963C2EEE6D5198C97ABE9FD9E0D32FB54D29601A404CA5706655E92E459C49A94DE9ABEA95C9596C5DF8983831264446F984",
		"vkiArr": [
			"575AD1B299E5F2D6C54D27CEEAB2D540836712C7F3C42182FB49D33089FE1BDE342322611D066F212D3054212D6CA99B07E522705575346C4CB5790591DEADD5B965D924C0AA8575D0C547D141EDCDCF46103DEB12E5A48A869A265B42CA5CF5B0A90F3B9AE054472E3D5473B27E19C47088B2DD642478600B42C0FEE6540306",
			"2CC2B5E6B76BB981BA58C9F188ED4B8CC9D3194A3A89B39F8D4A1D401ABEBCD26335AD77D6E3A6BEBEB570F340BCCFEA1A5BF236B91C026AFA3A6506411FB5DD25FB43C768EF2C97358B7D2184DDF9BB05376847F6ADA019214E151DE198668C57F1FD37E64AF7FAA2DB32EDE6AF99E2668C92175C5DB2DDC76DEEB9AA46D771",
			"250FF864469C55AC00536A604D8ED738FBD603006AD729FF957601CCA01B24BF40901569C05B0D2B76A6C5A417670663DF5A34CECC96E67406547B35E65C5031DF5C22394B8CE2535A3724EAD084919C84FF6B5ED4594C66F9CCE5B3AE0031DA0C1A58B806ED366BB8A819AFB56F25DC2080792FDF2CEAA46DDE71A1D2D986E0"
		]
	},
	"key_shard": {
		"index": 1,
		"private_key_shard": "17E72B0D465C2391A55DEB3261791F8950D60E733FA4CD94DC6CED10D9436A8713B241AA11B4DC8F4136872C5C212B0835110A66D47C151552A999C0AAB0B96125974046BD9E3E2899AA58AB32099D192393FF5706B938D7DCEDD4672B72C765D202E239196EE7BF9A5FBC1E9EE0A1311F8485596719AD106B088541A1A77120"
	}
}
```



## Query Request Details

### Interface URL Example
> http://YOUR_HOST//arweave/query_key_shard

### Request Method
> POST

### Content-Type
> JSON

### Request Body

Parameter |   Type   | Required  | Description
:---: |:--------:|:---------:| :---:
pubkey_list_hash |  String  |    YES    | The hash of the concatenation of the public keys from `userPublicKeyList` generated by SHA-256 in hexadecimal.

```json
{
  "pubkey_list_hash": "f6fca699eae08f95900e06744487f421a9e516c896efb72af4a7aad375c30636"
}
```

#### Example of the success response

```json
{
  "success": true,
  "alive_time_seconds": 10,
  "k": 2,
  "l": 3,
  "key_length":4096
}
```

#### Example of failure response

```json
{
  "success": false
}
```




# Intel® SGX Remote Attestation

In this project, we use Intel SGX DCAP to support remote attestation to provide a certification that this service is securely running within an enclave on an Intel® SGX enabled platform. As the Response Result shown above, there is a field `tee_report` which is the remote attestation certification (the report) generated by the Intel® SGX enabled platform on which the service is currently running.

Therefore, we offer a simple verification demo [sgx-arweave-client-js](https://github.com/Safeheron/sgx-arweave-client-js.git) for this specific project to verify the validation of the remote attestation certification (the report).

For more details on DCAP and remote attestation, please refer to the [Intel® Documentation](https://download.01.org/intel-sgx/sgx-dcap/1.14/linux/docs/).




# Contributions & Contact

This library is maintained by Safeheron. Contributions are welcomed! Besides, for GitHub issues and PRs, feel free to reach out by email.
