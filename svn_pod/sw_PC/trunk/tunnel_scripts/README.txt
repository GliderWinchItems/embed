/* File: README.txt -- for the tunnel_scripts directory
 */

There are 3 tunnel scripts: tunnel-startup and tunnel-shutdown do the
obvious functions.  Tunnel-running determines if the tunnel is up or
not and prints a message on stdout.  Terminal-running can be used with
the "-z" switch to get it to print out just the pid.  This is useful
within other scripts.

If you have a Linux box that has access to the FIT PC (via ssh), then
you can start the tunnel from the FIT PC to your Linux box.  An
administrator logged into the FIT PC can log into your Linux box with
the following command:

    $ ssh -p 22222 localhost
    
Note that tunnel-startup assumes you have an account on the FIT PC with
the same account name as you're logged into on your Linux box.

--

09/10/2012
More notes:

Three machines involved:
remote machine ubuntu
FIT server 
my machine deh

Remote machine starts tunnel--
tunnel-startup

This connects to the FIT server, ubuntu account.  For this to work the
remote machine must have the a rsa public key added "authorized_keys" in
FIT PC, /home/ubuntu/.ssh directory.

My machine ssh's into the FIT PC ubuntu account--
ssh -p 41574 -l ubuntu $FITIP

For this to work, "my machine" must have the private key in /home/deh/.ssh
and the FIT PC must have the public key of the pair in /home/ubuntu/,ssh
added to "authorized_keys".

My machine logged into the FIT PC as ubuntu, then connects to the tunnel
ssh -p 22222 localhost

If successful it asks for the password of the remote machine, e.g.
DATA LOGGER

If the "known_hosts" changed, then the offending "line" from the "known_hosts"
file must be deleted.  The next attempt will then ask for a yes/no to allow and
then, given a yes, adds the host. and the remote host password will be requested.

09/12/2012

Executing svn (or ssh) on remote machine--

For svn to work the remote machine has to have ~/.subversion/config
file changed so that the rsa key specified (-i switch).  It doesn't pick up the default when 
ssh is initiated via the reverse ssh tunnel.  

Example--
sshtunnelFIT = ssh -q -p 41574 -l deh -i /home/deh/AOA150-1

sshtunnelFIT = ssh -q -p 41574 -l deh [Does NOT work]
sshtunnelFIT = ssh -q -p 41574 -l deh -i ~/AOA150-1 [Does NOT work]

To ssh out of the remote machine back into the FIT--
ssh -p 41574 -l deh $FITIP -i /home/deh/AOA150-1





