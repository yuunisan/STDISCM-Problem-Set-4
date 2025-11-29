### DistributedOCR

## Prerequisites
  Virtual Machine (VM): Windows VM (or separate physical machine) for the server.
  Host Machine: Your local computer for the client.
  Network: The Host and the VM must be on the same network or able to ping each other.

  Note: If using VirtualBox/VMware, ensure the Network Adapter is set to "Bridged Adapter" so they can communicate.

## How to Run
  Step 1: Start the Server (Virtual Machine)
  Log into your separate computer or Virtual Machine.
  Navigate to the directory containing the server executable.
  Launch the application:
    OCRServer.exe

  Important: Note the IP address of this VM (run ipconfig in the command prompt). You will likely need this to configure the client.
  Ensure the server indicates it is "Listening" or "Ready".

  Step 2: Start the Client (Local Machine)
  On your main computer, navigate to the client folder.
  Launch the application:
    OCRClient.exe

  If prompted for a destination IP, enter the IP address of the VM from Step 1.
