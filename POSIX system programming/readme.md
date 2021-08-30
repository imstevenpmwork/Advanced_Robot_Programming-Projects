# Advanced Robot Programming Assigment 2019/2020 V2.0

## Instructions for use

The following instructions were tested using Ubuntu Focal Fossa 20.04.1 LTS on August 6th, 2020.

Open a new terminal in a chose directory and clone the repository given in [3] by running:
```git clone https://github.com/imstevenpm/ARP_Assigment.git```
    
In the same shell, execute the bash script by running:
```./Bash_StevenPalma.sh```

A new terminal will show up and the SnPnLn and Gn executables will be created in the current directory.
    
Before continuing, the user can set the parameters from the Config_StevenPalma.config file to a different ones. In the new terminal, execute the SnPnLn executable by running:
```./SnPnLn```

The network should initialize by showing on shell the PID of each Posix process created. Also, a time delay of 10 seconds is waited before Gn sends the first message to Pn and the communication cycle starts.
    
After the 10 seconds, messages from all the Posix processes will appear in the shell indicating their current status. The network is up and running. Notice that the Log_StevenPalma.log file is created as well as the FIFO files for the nammed pipes in the current directory of the user's machine.
    
From here, -in a different shell- the user can send the console signals to Sn for displaying the Log_StevenPalma.log content by running:
```kill -SIGUSR2 <Sn PID>```

or for starting/stopping Pn from receiving messages by running:
```kill -SIGUSR1 <Sn PID>```
    
To end the execution of the network the user can type CTRL+C

For more information, please refer to the report.

## Authors
* Steven Palma Morera: imstevenpm.study@gmail.com
