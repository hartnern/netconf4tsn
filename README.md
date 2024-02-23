# netconf4tsn
This project provides a sysrepo daemon that configures a network interface card (nic) with TSN configuration taken from a datastore.

# Dependencies
install the [netopeer2](https://github.com/CESNET/netopeer2) project according to the installation guide.

The following YANG modules need to be installed. Start a netopeer2 server first, using `sudo netopeer2-server`, then run the install_modules.sh script.
- [iana-if-type@2023-01-26](https://www.iana.org/assignments/yang-parameters/iana-if-type@2023-01-26.yang)
- [ieee802-dot1q-bridge@2023-10-26](https://github.com/YangModels/yang/blob/cf5375bfe2a7912bf3936b79c5abf320ffebd0e3/standard/ieee/published/802.1/ieee802-dot1q-bridge.yang)
- [ieee802-dot1q-sched-bridge@2023-10-26](https://github.com/YangModels/yang/blob/cf5375bfe2a7912bf3936b79c5abf320ffebd0e3/standard/ieee/published/802.1/ieee802-dot1q-sched-bridge.yang)
- [ieee802-dot1q-sched@2023-10-22](https://github.com/YangModels/yang/blob/cf5375bfe2a7912bf3936b79c5abf320ffebd0e3/standard/ieee/published/802.1/ieee802-dot1q-sched.yang) with the scheduled-traffic feature enabled
- [ieee802-dot1q-types@2023-10-26](https://github.com/YangModels/yang/blob/cf5375bfe2a7912bf3936b79c5abf320ffebd0e3/standard/ieee/published/802.1/ieee802-dot1q-types.yang)
- [ieee802-types@2023-10-22](https://github.com/YangModels/yang/blob/cf5375bfe2a7912bf3936b79c5abf320ffebd0e3/standard/ieee/published/802/ieee802-types.yang)
- [ietf-interfaces@2018-02-20](https://github.com/YangModels/yang/blob/cf5375bfe2a7912bf3936b79c5abf320ffebd0e3/standard/ietf/RFC/ietf-interfaces%402018-02-20.yang)


# How to use
Depending on the nic you plan to use, you will most likely want to change the defines in daemon.c to fit your specifications.

1. start the netopeer2 server
```
sudo netopeer2-server
```
2. start the netopeer2 client and connect to the server
```
netopeer2-cli
connect
```
3. compile and start the daemon
```
make
sudo ./daemon
```


The daemon will now detect any changes made in the running datastore and send an according tc command to the nic.



