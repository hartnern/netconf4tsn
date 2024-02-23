mkdir modules
cd modules

wget https://bit.ly/ietf-interfaces \
https://bit.ly/ieee802-types \
https://bit.ly/ieee802-dot1q-types \
https://bit.ly/ieee802-dot1q-sched \
https://bit.ly/ieee802-dot1q-sched-bridge \
https://bit.ly/ieee802-dot1q-bridge \
https://bit.ly/iana-if-type

for file in *; do mv "$file" "$file.yang"; done

sudo sysrepoctl -i ietf-interfaces.yang
sudo sysrepoctl -i iana-if-type.yang
sudo sysrepoctl -i ieee802-types.yang
sudo sysrepoctl -i ieee802-dot1q-types.yang
sudo sysrepoctl -i ieee802-dot1q-sched.yang
sudo sysrepoctl -i ieee802-dot1q-bridge.yang
sudo sysrepoctl -i ieee802-dot1q-sched-bridge.yang

sudo sysrepoctl -c ieee802-dot1q-sched -e scheduled-traffic
