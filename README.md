# Project
- Name : Common-Communicator

### Simple-Description
- It's SDK library that support Transaction-orient(like as General UDP/TCP) and Service-orient(like as vSomeIP).
- Objectives : Develop common-framework for communication with another device and another processor and cloud-server.

### Features
Feature-name | Support | Category | Description
:---|:---:|:---:|:---
`UDP` | O | `Provider` | **[_Transaction_]** Fundamental UDP server/client.
`TCP` | O | `Provider` | **[_Transaction_]** Fundamental TCP server/client.
`UDS-UDP` | O | `Provider` | **[_Transaction_]** Fundamental UDP server/client base on UDS.
`UDS-TCP` | O | `Provider` | **[_Transaction_]** Fundamental TCP server/client base on UDS.
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
- version 0.4.0 (Date: 2022-08-06)
---
## Installation
> Please refer following commands.
> So, you can see the SDK library(libcommunicator.so) in api folder.
```shell
$ cd ${work}
$ bash ./build.sh -m debug -t clean -arch x86
$ bash ./build.sh -m debug -t comm -arch x86
$ bash ./build.sh -m debug -t protocol -arch x86
$ bash ./build.sh -m debug -t example -arch x86
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
   $ cd ${work}/api/include
   $ cat ICommunicator.h
   ```
   - Common-API is described in "ICommunicator.h file".

### Example (UDP)
- You can test the communicator SDK library by using following guide-line.
   > If you want to make a application with common-API, then reference following guide-line.
   ```shell
   $ cd ${work}
   $ bash ./build.sh -m debug -t example -arch x86
   $ ./debug/bin/sample_udp_server ${Path-of-desp-Alias.json} ${Path-of-desp-Protocol.json} &
   $ ./debug/bin/sample_udp_client ${Path-of-desp-Alias.json} ${Path-of-desp-Protocol.json} 
   ```
   - Attention : You have to set "LD_LIBRARY_PATH" with "${work}/debug/lib".

### Example (TCP)
- You can test the communicator SDK library by using following guide-line.
   > If you want to make a application with common-API, then reference following guide-line.
   ```shell
   $ cd ${work}
   $ bash ./build.sh -m debug -t example -arch x86
   $ ./debug/bin/sample_tcp_server ${IP} ${Port} ${Path-of-desp-Alias.json} ${Path-of-desp-Protocol.json} &
   $ ./debug/bin/sample_tcp_client ${Path-of-desp-Alias.json} ${Path-of-desp-Protocol.json} ${Server-IP} ${Server-Port}
   ```
   - Attention : You have to set "LD_LIBRARY_PATH" with "${work}/debug/lib".

### Example (UDS with UDP protocol)
- You can test the communicator SDK library by using following guide-line.
   > If you want to make a application with common-API, then reference following guide-line.
   ```shell
   $ cd ${work}
   $ bash ./build.sh -m debug -t example -arch x86
   $ ./debug/bin/sample_uds_udp_server ${IP} ${Port} ${Path-of-desp-Alias.json} ${Path-of-desp-Protocol.json} &
   $ ./debug/bin/sample_uds_udp_client ${Path-of-desp-Alias.json} ${Path-of-desp-Protocol.json}
   ```
   - Attention : You have to set "LD_LIBRARY_PATH" with "${work}/debug/lib".

### Example (UDS with TCP protocol)
- You can test the communicator SDK library by using following guide-line.
   > If you want to make a application with common-API, then reference following guide-line.
   ```shell
   $ cd ${work}
   $ bash ./build.sh -m debug -t example -arch x86
   $ ./debug/bin/sample_uds_tcp_server ${Path-of-desp-Alias.json} ${Path-of-desp-Protocol.json} &
   $ ./debug/bin/sample_uds_tcp_client ${Path-of-desp-Alias.json} ${Path-of-desp-Protocol.json}
   ```
   - Attention : You have to set "LD_LIBRARY_PATH" with "${work}/debug/lib".

---
### TBD-list
- Build Routing-Manager.
- Support vSOME/IP.
- Support HTTP for OAuth2 authentication.
- Common-API will be improved through review-processing & consideration variety of situation.
