# Project
- Name : Common-Communicator

### Simple-Description
- It's SDK library that support Transaction-orient(like as General UDP/TCP) and Service-orient(like as vSomeIP).
- Objectives : Develop common-framework for communication with another device and another processor and cloud-server.

### Features
Feature-name | Support | Category | Description
:---|:---:|:---:|:---
`UDP` | O | `Server/Client` | **[_Transaction_]** Fundamental UDP server/client.
`TCP` | O | `Server/Client` | **[_Transaction_]** Fundamental TCP server/client.
`CPBigEndian` | O | `Protocol` | Sample protocol for Big-Endian type.
`CPLittleEndian` | O | `Protocol` | Sample protocol for Little-Endian type.
`Protocol-Chain` | O | `Protocol` | Chaining for Customized-Protocol cascading.
`PAL` | O | `Protocol` | Protocols Abstraction Layer.
`Alias` | O | `Addressing` | Friendly, naming with regard to address on TCP/UDP/vSomeIP/IoTivity.
`Common-API` | X | `API` | Support **_Restrict_** API about only for transaction-oriented communication.

## License
```
Copyright [2019-] 
Written by EunSeok Kim <es.odysseus@gmail.com>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```

### Developer
- esOdysseus (email: es.odysseus@gmail.com)

### Latest Release
- version 0.1.8 (Date: 2020-05-01)
---
## Installation
> Please refer following commands.
> So, you can see the SDK library(libcommunicator.so) in api folder.
```shell
$ cd ${work}
$ bash ./build.sh -m release -t clean -arch x86
$ bash ./build.sh -m release -t comm -arch x86
$ bash ./build.sh -m release -t protocol -arch x86
$ bash ./build.sh -m release -t example -arch x86
```
### Library Dependency
- glibc     : for socket communication.
- dl        : for dynamic-link to protocol-library.
- pthread   : for create pthread.
- rapidjson : for json file read/write. (include header-files to "lib/json/rapidjson")
    > reference-Site: https://github.com/Tencent/rapidjson/blob/master/example/tutorial/tutorial.cpp
---
## Common-API
   > API for loose-dependency between Middleware-Communication library and Application.
   ```shell
   $ cd ${work}/api/common_api
   $ cat IAppInf.h
   ```
   - Common-API is described in "IAppInf.h file".

### Example (UDP)
- You can test the communicator SDK library by using following guide-line.
   > If you want to make a application with common-API, then reference following guide-line.
   ```shell
   $ cd ${work}
   $ bash ./build.sh -m release -t example -arch x86
   $ ./release/bin/sample_udp_server ${IP} ${Port} ${Path-of-desp-Protocol.json} ${Path-of-desp-Alias.json} &
   $ ./release/bin/sample_udp_client ${Path-of-desp-Protocol.json} ${Path-of-desp-Alias.json}
   ```
   - Attention : You have to set "LD_LIBRARY_PATH" with "${work}/release/lib".

### Example (TCP)
- You can test the communicator SDK library by using following guide-line.
   > If you want to make a application with common-API, then reference following guide-line.
   ```shell
   $ cd ${work}
   $ bash ./build.sh -m release -t example -arch x86
   $ ./release/bin/sample_tcp_server ${IP} ${Port} ${Path-of-desp-Protocol.json} ${Path-of-desp-Alias.json} &
   $ ./release/bin/sample_tcp_client ${Path-of-desp-Protocol.json} ${Path-of-desp-Alias.json}
   ```
   - Attention : You have to set "LD_LIBRARY_PATH" with "${work}/release/lib".

---
## History
Date | Commit-ID | Version | Description
:----|:----:|:----:|:----
`2019-12-4` | `e5d31e09073be75c884eca4bd207b6e2c2be6895` | Ver 0.1.0 | First commit for Common-Communicator.
`2019-12-5` | `d37fd441949617a92e0fd583a703f56db21cfacf` | Ver 0.1.1 | Change folder-tree and API-design change.
`2019-12-7` | `ad863961333ca7c4e7bca054b04f39dfefa7fdb7` | Ver 0.1.1 | 1. Add Logger.<br> 2. UDP connected-call-back op enable.<br> 3. Remove 'using namespace std'.
`2020-01-5` | `63c626078a5a3631e467c5dfeef98ec90a13d426` | Ver 0.1.1 | 1. Decomposition of Protocol-library.<br> 2. Decomposition common-library.<br> 3. Create CConfigProtocol.
`2020-01-11` | `0ea787e9f1be03c0073dec341d131ef4c1d9c373` | Ver 0.1.2 | 1. Protocol-Chain Impled. <br> 2. Isolate Protocol-Lib & Communicator Lib. <br> 3. Apply PAL-concept for Dynamic-load Protocol-Library.
`2020-01-14` | `1ed4cff327dfd6b3b543ae1f65acaafc4596e1e6` | Ver 0.1.2 | Support ARMv7 Build.
`2020-02-01` | `1615a31aa20f49d56ab6dfb0df07fb08ef7dcbe3` | Ver 0.1.3 | 1. Support ARMv7 & Aarch64 Build.<br> 2. Done processing of default-argument.
`2020-02-02` | `7d763ab057b0532388a787f7ffd5edd8c9995cb9` | Ver 0.1.3 | Add Converter that convert alias-name to essential-address.
`2020-02-03` | `be28532ec0b07dfc337f0b9785216d22db5c6552` | Ver 0.1.3 | Add CConfigAliases class that load desp-alias.json file.
`2020-02-04` | `c9893217d54b6be28ec6f2f029bacf1c9e6464a9` | Ver 0.1.3 | 1. Support Client of UDP.<br> 2. Fix bug of _flag_op_ in CPayload class.
`2020-02-07` | `0cc7bf9e6d5427d2d5d5add075d90cfea644ab62` | Ver 0.1.4 | 1. Support Client of TCP.<br> 2. Support Server of TCP.<br> 3. Support Server of UDP.
`2020-02-12` | `30f2973d546a1620c2b49488e3fbecb316d827ba` | Ver 0.1.5 | 1. Add shared_mutex library for c++11.<br> 2. Add Address to Alias mapper.
`2020-04-03` | `0fdbdd8b2e4457fbe0440071002537b57b4b04bc` | Ver 0.1.6 | Add quit API.
`2020-04-18` | `bec7426aa13923a9017a7389399060f0c9318031` | Ver 0.1.7 | 1. Add connect, disconnect API.<br> 2. Allow additional new-alias in runtime.
`2020-05-01` | `` | Ver 0.1.8 | 1. Bug-Fix: TCP disconn & re-connect bug.<br> 2. Bug-Fix: Protocol-Chain buf.<br> 3. Bug-Fix: Random-Device bug.<br> 4. API-chang: connect --> connect_try.

### TBD-list
- Build Routing-Manager.
- Support vSOME/IP.
- Support IoTivity of OCF.
- Support UDS for sharing state of communicator.
- Common-API will be improved through review-processing & consideration variety of situation.
