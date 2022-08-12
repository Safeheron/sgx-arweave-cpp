
#### Interface URL
> http://YOURHOST/arweave/create_key_shard

#### 请求方式
> POST

#### Content-Type
> JSON

#### Request Body
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
	"callBackAddress": "http://127.0.0.1:8008/sayHi"
}
```
Parameter | Type | Required | Description
:-----: |:-------:|:--:| :---
userPublicKeyList | String  |  YES | This field represents the user public key list, which collects the ECDSA public key of each user. Users can decrypt the encrypted key shard information by using their private keys corresponding to the public keys.
k | Integer |  YES | Threshold numerator.
l | Integer |  YES | Threshold denominator.
keyLength | Integer |  YES | The length of the private key to be generated. Only in 1024, 2048, 3072 and 4096.
webhookURL | String  |  YES | Webhook address. The generated results will be called back to this address via a webhook service.

#### 成功响应示例
```json
{
	"success": true,
	"message": "Request has been accepted."
}
```
参数名 |  参数类型   | 参数描述
:---: |:-------:| :----:
success | Boolean | 1
message | String  | 1
#### 错误响应示例
```json
{
	"success": false,
	"message": "Error Message"
}
```
参数名 |  参数类型   | 参数描述
:---:|:-------:| :---
success | Boolean | 1
message | String  | 1
