# Setting Up FTP with Docker on Google Cloud Platform

This guide explains how to configure FTP and Docker on a Google Cloud Platform (GCP) virtual machine (VM), ensuring both services run seamlessly together.

---

## Steps to Configure FTP and Docker

### 1. Stop Docker
Stop the Docker service to avoid interference during configuration:
```bash
sudo systemctl stop docker
```

---

### 2. Flush iptables Rules
Clear all existing iptables rules:
```bash
sudo iptables -F
sudo iptables -X
sudo iptables -t nat -F
sudo iptables -t nat -X
sudo iptables -t mangle -F
sudo iptables -t mangle -X
sudo iptables -t raw -F
sudo iptables -t raw -X
```

---

### 3. Modify Docker Configuration
Prevent Docker from managing iptables by editing its configuration file:
```bash
sudo nano /etc/docker/daemon.json
```

Add the following configuration:
```json
{
  "iptables": false,
  "runtimes": {
    "nvidia": {
      "path": "nvidia-container-runtime",
      "runtimeArgs": []
    }
  }
}
```

Save the file (`Ctrl + X`, then `Y`, and press `Enter`).

---

### 4. Inspect Docker Networking
Verify Docker's primary network interface and subnet:
1. **Check Docker's network interface**:
    ```bash
    ip a
    ```
   Look for the interface named `docker0`.
2. **Check Docker's subnet**:
    ```bash
    docker network inspect bridge
    ```

---

### 5. Add iptables Rules for Docker Networking
Replace `<interface>` with your primary network interface (e.g., `ens5` or `eth0`), and `<subnet>` with Docker's subnet (e.g., `172.17.0.0/16`):
```bash
sudo iptables -I FORWARD -i docker0 -o <interface> -j ACCEPT
sudo iptables -I FORWARD -i <interface> -o docker0 -j ACCEPT
sudo iptables -t nat -A POSTROUTING -s <subnet> -j MASQUERADE
```

---

### 6. Configure FTP Passive Ports in iptables
Add rules to allow FTP traffic:
```bash
sudo iptables -A INPUT -p tcp --dport 21 -j ACCEPT
sudo iptables -A INPUT -p tcp --dport 10000:10100 -j ACCEPT
```

---

### 7. Configure vsftpd
Edit the `vsftpd` configuration file to set the passive IP address and ports:
```bash
sudo nano /etc/vsftpd.conf
```

Add or modify these lines:
```plaintext
pasv_enable=YES
pasv_min_port=10000
pasv_max_port=10100
pasv_address=<your-server-external-ip>
```

Save the file (`Ctrl + X`, then `Y`, and press `Enter`).

Restart the `vsftpd` service:
```bash
sudo systemctl restart vsftpd
```

---

### 8. Persist iptables Rules
Save the iptables rules to make them persistent across reboots:
```bash
sudo mkdir -p /etc/iptables
sudo iptables-save > /etc/iptables/rules.v4
```

Verify the saved rules:
```bash
cat /etc/iptables/rules.v4
```

Install the `iptables-persistent` package:
```bash
sudo apt install iptables-persistent
```

---

### 9. Start Docker
Restart the Docker service:
```bash
sudo systemctl start docker
```

---

### 10. Configure Google Cloud Console Firewall
To allow FTP traffic through the Google Cloud firewall:
1. Open **Google Cloud Console**.
2. Go to **VPC Network** > **Firewall Rules**.
3. Add a new firewall rule:
    - **Name**: Allow-FTP
    - **Targets**: All instances in the network (or specify your VM)
    - **Source IP ranges**: 0.0.0.0/0 (or restrict to specific IPs for security)
    - **Protocols and Ports**:
        - **TCP**: `21, 10000-10100`
4. Save the rule.

---

## Congratulations
Your GCP VM is now configured to run both FTP and Docker without conflicts. 🎉