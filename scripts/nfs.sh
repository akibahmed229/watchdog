#!/usr/bin/env bash

server_path=$1
server_ip_address=$2

client_path=$3
client_ip_address=$4

# check if command line arguments are provided 
if [ $# -le 3 ]; then
    echo "Please provide the server path, server IP address, client path, and client IP address"
    echo "--------------------------------------"
    echo "--------------------------------------"
    echo "Usage: nfs.sh <server_path> <server_ip_address> <client_path> <client_ip_address>"
    exit 1
fi

function server_configuration() {
    # Install NFS server
    sudo apt install -y nfs-kernel-server rpcbind nfs-common ndsidmap

    # enable and start the nfs-server service 
    sudo systemctl enable nfs-server rpcbind 
    sudo systemctl start nfs-server rpcbind rpc-statd nfs-idmap

    # Add the port to the firewall 
    sudo ufw allow nfs 
    sudo ufw allow mountd
    sudo ufw allow rpc-bind 

    # Create a shared directory & setup the permissions
    sudo mkdir -p $server_path
    sudo chown nobody:nogroup $server_path
    sudo chmod 777 $server_path

    # Export the shared directory 
    sudo echo "$server_path $client_ip_address(rw,sync,no_root_squash,no_subtree_check)" >> /etc/exports
    exportfs -rv
}

function client_configuration(){
    # Install NFS client
    sudo apt install -y nfs-kernel-server rpcbind

    # Enable and start the nfs-client service 
    sudo systemctl enable rpcbind
    sudo systemctl start rpcbind

    # Add the port to the firewall 
    sudo ufw allow nfs 
    sudo ufw allow mountd 
    sudo ufw allow rpc-bind 
    
    # Create a directory to mount the shared directory 
    sudo mkdir -p $client_path

    # Mount the shared directory 
    sudo mount $server_ip_address:$server_path $client_path
}

read -rp "Configure the server or client
1. Server
2. Client

0. Exit
" choice


case $choice in
    1)
        server_configuration
        ;;
    2)
        client_configuration
        ;;
    0)
        exit 0
        ;;
    *)
        echo "Invalid choice"
        ;;
esac
