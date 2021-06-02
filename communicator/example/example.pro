TARGET = communicator_example
TEMPLATE = subdirs

CONFIG += ordered
SUBDIRS += sample_tcp_client    \
           sample_tcp_server    \
           sample_udp_client    \
           sample_udp_server    \
           sample_uds_tcp_client    \
           sample_uds_tcp_server    \
           sample_uds_udp_client   \
           sample_uds_udp_server

