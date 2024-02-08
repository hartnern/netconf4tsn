# netconf4tsn
This project provides a sysrepo daemon that configures a network interface card (nic) with TSN configuration taken from a datastore.

# Dependencies
install the [netopeer2](https://github.com/CESNET/netopeer2) project according to the installation guide.

Use `sysrepoctl -i MODULEPATH` to install the following [TSN modules](https://github.com/YangModels/yang/tree/main/standard/ieee/published/802.1) for the netopeer2 server. 

- iana-if-type@2023-01-26
- ieee802-dot1q-bridge@2023-10-26
- ieee802-dot1q-sched-bridge@2023-10-26
- ieee802-dot1q-sched@2023-10-22
- ieee802-dot1q-types@2023-10-26
- ieee802-types@2023-10-22
- ietf-interfaces@2018-02-20

For ieee802-dot1q-sched enable the scheduled-traffic feature using `sysrepoctl -c ieee802-dot1q-sched -e scheduled-traffic`.

# How to use
Depending on the nic you plan to use, you will most likely want to change the defines in daemon.c to fit your specifications.

1. start the netopeer2 server
2. start the netopeer2 client and connect to the server
3. start the daemon

The daemon will now detect any changes made in the running datastore and send an according tc command to the nic.


