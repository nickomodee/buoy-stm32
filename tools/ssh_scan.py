import nmap

def scan_network(network):
    nm = nmap.PortScanner()
    nm.scan(hosts=network, arguments='-p 22 --open')
    
    ssh_hosts = []
    for host in nm.all_hosts():
        if nm[host].has_tcp(22) and nm[host]['tcp'][22]['state'] == 'open':
            ssh_hosts.append(host)
    
    return ssh_hosts

network_range = '192.168.68.0/24'
ssh_enabled_hosts = scan_network(network_range)

print("SSH-enabled hosts:")
for host in ssh_enabled_hosts:
    print(host)