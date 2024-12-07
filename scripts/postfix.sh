#!/usr/bin/env bash


mail=$1
password=$2
hostname=$(hostname -f)

# check if the user has provided the email and password 
if [ -z "$mail" ] || [ -z "$password" ] && [ $# -le 2 ]; then
    echo "Please provide your email and password"
    echo "--------------------------------------"
    echo "--------------------------------------"
    echo "Usage: postfix.sh <email> <password>"
    exit 1
fi

# Install Postfix and mailutils 
sudo apt-get install -y postfix mailutils neovim


function setup_postfix() {
    # Set up Postfix to use Gmail as a relay host 
    sudo postconf -e 'relayhost = [smtp.gmail.com]:587'
    sudo postconf -e 'myhostname= $hostname'
    
    # Location of sasl_passwd we saved
    sudo postconf -e "smtp_sasl_password_maps = hash:/etc/postfix/sasl/sasl_passwd"
    
    # Enables SASL authentication for postfix
    sudo postconf -e 'smtp_sasl_auth_enable = yes'
    sudo postconf -e 'smtp_tls_security_level = encrypt'
    
    # Disallow methods that allow anonymous authentication
    sudo postconf -e 'smtp_sasl_security_options = noanonymous'
}

function setup_postfix_passwdFile() {
    # Create the sasl_passwd file
    sudo touch /etc/postfix/sasl/sasl_passwd
    
    # Add Gmail credentials to the sasl_passwd file 
    echo "[smtp.gmail.com]:587 $mail:$password" | sudo tee -a /etc/postfix/sasl/sasl_passwd > /dev/null

    
    # Change the permissions of the sasl_passwd file
    sudo chmod 600 /etc/postfix/sasl/sasl_passwd
    sudo chown root:root /etc/postfix/sasl/sasl_passwd
    
    # Update Postfix to use the sasl_passwd file and delete the plaintext file
    sudo postmap /etc/postfix/sasl/sasl_passwd 
    sudo rm /etc/postfix/sasl/sasl_passwd
}


# Set up Postfix 
setup_postfix 
setup_postfix_passwdFile

# Restart postfix
sudo systemctl restart postfix
