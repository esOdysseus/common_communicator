# Project
- Name : Common-Communicator
---
## Simple-Description
- It's SDK library that support Transaction-orient(like as General UDP/TCP) and Service-orient(like as vSomeIP).
- Objectives : Develop common-framework for communication with another device and another processor and cloud-server.
---
### Features
Feature-name | Support | Category | Description
:---|:---:|:---:|:---
`UDP` | O | `Server/Client` | **[_Transaction_]** Fundamental UDP server/client.
`TCP` | O | `Server` | **[_Transaction_]** Fundamental TCP server.
`CPBigEndian` | O | `Protocol` | Sample protocol for Big-Endian type.
`CPLittleEndian` | O | `Protocol` | Sample protocol for Little-Endian type.
`Common-API` | X | `API` | **_Restrict_** API about only server for transaction-oriented communication.
---
### License
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
---
### Developer
- esOdysseus (email: es.odysseus@gmail.com)
---
### Latest Release
- version 0.1.0 (Date: 2019-12-03)
---
## Installation
> Please refer following commands.
> So, you can see the SDK library(libcommunicator.so) in api folder.
```shell
$ cd ${work}
$ bash ./build.sh
```
---
## Common-API
   > API for loose-dependency between Middleware-Communication library and Application.
   ```shell
   $ cd ${work}/api/common_api
   $ cat IAppInf.h
   ```
   - Common-API is described in "IAppInf.h file".
---
## Example
- You can test the communicator SDK library by using following guide-line.
   > If you want to make a application with common-API, then reference following guide.
   ```shell
   $ cd ${work}/example
   $ bash ./build.sh
   $ ./build/common_api/example_common_api ${IP} ${Port}
   ```
   - Attention : You have to set "LD_LIBRARY_PATH" with "${work}/api".
---
## TBD-list
- It will support SOME/IP protocol by common-API.
- Common-API will be improved through review-processing & consideration variety of situation.
