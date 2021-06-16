
# History

Date | Version | Description
:----|:----:|:----
`2019-12-4` | Ver 0.1.0 | First commit for Common-Communicator.
`2019-12-5` | Ver 0.1.1 | Change folder-tree and API-design change.
`2019-12-7` | Ver 0.1.1 | 1. Add Logger.<br> 2. UDP connected-call-back op enable.<br> 3. Remove 'using namespace std'.
`2020-01-5` | Ver 0.1.1 | 1. Decomposition of Protocol-library.<br> 2. Decomposition common-library.<br> 3. Create CConfigProtocol.
`2020-01-11` | Ver 0.1.2 | 1. Protocol-Chain Impled. <br> 2. Isolate Protocol-Lib & Communicator Lib. <br> 3. Apply PAL-concept for Dynamic-load Protocol-Library.
`2020-01-14` | Ver 0.1.2 | Support ARMv7 Build.
`2020-02-01` | Ver 0.1.3 | 1. Support ARMv7 & Aarch64 Build.<br> 2. Done processing of default-argument.
`2020-02-02` | Ver 0.1.3 | Add Converter that convert alias-name to essential-address.
`2020-02-03` | Ver 0.1.3 | Add CConfigAliases class that load desp-alias.json file.
`2020-02-04` | Ver 0.1.3 | 1. Support Client of UDP.<br> 2. Fix bug of _flag_op_ in CPayload class.
`2020-02-07` | Ver 0.1.4 | 1. Support Client of TCP.<br> 2. Support Server of TCP.<br> 3. Support Server of UDP.
`2020-02-12` | Ver 0.1.5 | 1. Add shared_mutex library for c++11.<br> 2. Add Address to Alias mapper.
`2020-04-03` | Ver 0.1.6 | Add quit API.
`2020-04-18` | Ver 0.1.7 | 1. Add connect, disconnect API.<br> 2. Allow additional new-alias in runtime.
`2020-05-01` | Ver 0.1.8 | 1. Bug-Fix: TCP disconn & re-connect bug.<br> 2. Bug-Fix: Protocol-Chain buf.<br> 3. Bug-Fix: Random-Device bug.<br> 4. API-chang: connect --> connect_try.
`2020-05-13` | Ver 0.1.9 | 1. Buf-Fix: Unintended multiple sending by E_KEEP_PAYLOAD_AFTER_TX option.<br> 2. Apply ThreadPool & queue for Rx.<br> 3. Upgrade Protocol API for receiving of multiple message.
`2020-05-29` | Ver 0.1.9 | 1. Add Cinet_uds class.<br> 2. Clean warning message.
`2020-05-30` | Ver 0.1.9 | Apply Cinet_uds class to TCP/UDP provider class.
`2020-05-31` | Ver 0.1.9 | Implement UDS TCP/UDP provider frame, but not yet test it.
`2020-06-01` | Ver 0.2.0 | Add Feature UDS TCP/UDP communication. & sample program to test UDS.
`2021-01-02` | Ver 0.2.1 | Definition of Resource-Concept.
`2021-05-18` | Ver 0.2.2 | [Protocol] Add UniversalCMD & improve structure of Protocol-Handling.
`2021-05-20` | Ver 0.2.3 | Improve ConfigAlias class.
`2021-05-22` | Ver 0.2.4 | [Bug-Fix] Fix memory-corruption of CRawMessage Class.<br> [Protocol] Change structure of protocol-header in UniversalCMD protocol.
`2021-06-02` | Ver 0.2.5 | Refactoring Code-Structure to apply for Alias-Policy pair of App-path & PVD-id.
`2021-06-05` | Ver 0.2.6 | Refactoring API-Structure to apply for API-Consistency.
`2021-06-05` | Ver 0.2.7 | Modify legacy Sample-Test Program to Variable Sample-Test.
`2021-06-10` | Ver 0.2.7 | Modify CPUniversalCMD & CRawMessage to support Zero-message Tx/Rx.
`2021-06-16` | Ver 0.2.8 | Adding Feature that Searching Provider-Alias which is connected with wanted peer.
