# Project
- Name : Common-Communicator

### Simple-Description
- It's SDK library that support Transaction-orient(like as General UDP/TCP) and Service-orient(like as vSomeIP).
- Objectives : Develop common-framework for communication with another device and another processor and cloud-server.

### Features
Feature-name | Support | Category | Description
:---|:---:|:---:|:---
`UDP` | O | `Server/Client` | **[_Transaction_]** Fundamental UDP server/client.
`TCP` | O | `Server` | **[_Transaction_]** Fundamental TCP server.
`CPBigEndian` | O | `Protocol` | Sample protocol for Big-Endian type.
`CPLittleEndian` | O | `Protocol` | Sample protocol for Little-Endian type.
`Common-API` | X | `API` | **_Restrict_** API about only server for transaction-oriented communication.

## License
```
Copyright [2019] [es.odysseus@gmail.com]

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
- version 0.1.1 (Date: 2019-12-05)
---
## Installation
> Please refer following commands.
> So, you can see the SDK library(libcommunicator.so) in api folder.
```shell
$ cd ${work}
$ bash ./build.sh release
```
### Library Dependency
- glibc     : for socket communication.
- dl        : for dynamic-link to protocol-library.
- stdc++11  : base library for overall-src.
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

### Example
- You can test the communicator SDK library by using following guide-line.
   > If you want to make a application with common-API, then reference following guide.
   ```shell
   $ cd ${work}
   $ bash ./build.sh release example
   $ ./release/bin/example_common_api ${IP} ${Port}
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

### TBD-list
- It will support SOME/IP protocol by common-API.
- Common-API will be improved through review-processing & consideration variety of situation.
