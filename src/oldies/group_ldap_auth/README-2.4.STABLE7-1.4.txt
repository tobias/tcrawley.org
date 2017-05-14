This README shipped with the 1.4 version of the group_ldap_auth patch for 
squid 2.4.STABLE7, and may not be accurate for any other version.

The group-ldap-auth module performs user and group authentication to an ldap 
server. It supports static and dynamic groups.


BUILD INSTRUCTIONS
------------------

This module requires the openldap libraries (http://www.openldap.org) to build.
The latest version of openldap that it has been test with is 2.0.25.
Once openldap has been installed and the group-ldap-auth patch has been applied, 
build with:
	
	% cd <patched-squid-src>
	% ./configure <configure-options>
	% make
	% make install
	% cd /auth_modules/GROUP_LDAP/	
	% make
	% make install


USAGE
-----

This module does not use the auth_module communication method, and requires 
squid 2.4.STABLE7-ldap_auth-1.4. The diff file you used to get his module patched 
2.4.STABLE7 source tree to 2.4.STABLE7-ldap_auth-1.4. If you received this module 
through some other means, go to http://group-ldap-auth.sourceforge.net/
to get the full diff.

Usage: ./group_ldap_auth [-v?] -h ldap-server [-p ldap-port] -b base-dn [-m group-member-attr] \
[-g group-name-attr] [-l log-file-prefix] [-s scope] [-f user-attribute-filter] [-o group-object-class] \
[-u uid-attr] [-d bind-dn] [-w bind-password] [-T | -S]
     -v                    Print version information and exit
     -?                    Print this usage information and exit
     -l log-file-prefix    Specify the debug logging file prefix. This prefix
                           should be the full path to the logging logging
                           location plus a filename. The pid of the process will
                           be appended to this prefix to create the filename.
                           Debug logging is off by default.
     -h ldap-server        Specify ldap server name (required)
     -p ldap-port          Specify ldap port (default: 389)
     -b base-dn            Specify base search db (required)
     -f filter             Specify user attribute filter. This filter is ANDed
                           with the <uid-attr>=<uid> filter for the initial
                           search for the user DN.
     -g group-name-attr    Specify the attribute to be searched using the group
                           name
                           (default: cn)
     -m group-member-attr  Specify the attribute used to associate users to
                           groups
                           (default: member)
     -o group-object-class Specify the objectclass that defines a group
                           (default: groupOfNames)
     -s scope              Specify ldap scope - one of: base, one, or sub
                           (default: sub)
     -u uid-attr           Specify the uid attribute
                           (default: uid)
     -d bind-dn            Specify the binddn for searching
                           (default: )
     -w bind-password      Specify the password for searching
                           (default: )
     -T            	       Use TLS 
                           (default: no)
     -S            	       Use SSL
                           (default: no)

The above options are specified along with the path to the module using the 
ldap_auth_program directive in squid.conf. example:

	ldap_auth_program /usr/local/squid/libexec/squid/group_ldap_auth -b \
   	   o=fatgut.org -h ldapserver -m uniqueMember -o groupOfUniqueNames -l \
       /tmp/auth_db_log

To tell squid to use ldap_auth, use the ldap_auth acl directive:

	acl aclname ldap_auth (((static | dynamic) group) | username) ...
	
Use REQUIRED to accept any valid username. A list of groups and/or users can 
be specified here. The group must be prefaced with either static or dynamic to 
define the group type. Group names with spaces in them should be enclosed by 
single quotes ("'"). Examples:
	
	acl user_acl ldap_auth REQUIRED
	- requires the user to be in the ldap db, and the password to be valid
	
	acl users_acl ldap_auth joed janed toc
	- requires the user to be one of {joed, janed, toc}, the user to be 
	  in the ldap db, and the password to be valid

	acl group_acl ldap_auth static 'product managers' dynamic uid=j*
	- requires the user to be a member of the static product managers group
	  or a member of the dynamic uid=j* group (all user id's starting with j). 
	  The user must also exist in the ldap db and have a valid password.

Users and groups can be mixed in an acl statement, but the special REQUIRED 
parameter cannot be mixed. Examples:
	
	acl users_groups_acl ldap_auth frank john joe static 'useful managers' 
	*VALID*

	acl invalid_acl ldap_auth REQUIRED frank john joe
	*INVALID*

	
There are other configuration options that affect ldap_auth:

ldap_auth_children
	
	The number of ldap authenticator processes to spawn (default 5). 
	If you start too few Squid will have to wait for them to process 
	a backlog of user/password verifications, slowing it down. When 
	password verifications are done via a slow network or you have 
	ldap_auth acls with several groups you are likely to need lots of 
	ldap authenticator processes.

ldap_auth_cache_size

	The size of the ldap username/password/group cache in entries (default 64).

ldap_auth_cache_ttl

	The number of seconds a checked ldap username/password/group combination 
	remains cached (default 3600). If a wrong password is given for a cached 
	user, the user gets removed from the username/password/group cache forcing
	a revalidation.

ldap_auth_cache_ip_ttl

	With this option you control how long a ldap authentication
	will be bound to a specific IP address. If a request using
	the same user name is received during this time then access
	will be denied and both users are required to reauthenticate
	them selves.  The idea behind this is to make it annoying
	for people to share their password to their friends, but
	yet allow a dialup user to reconnect on a different dialup
	port.

    If the TTL is set to a negative value then the check is
    strict, completely denying access from other IP addresses
    until the TTL has expired.

	The default is 0 to disable the check. Recommended value
	if you have dialup users are no more than 60 (seconds). If
	all your users are stationary then higher values may be
	used.


HOW IT WORKS
------------

The module communicates with squid though the squid helper system: squid writes
data to the modules stdin, the module writes results back to its stdout. The 
other auth modules use a simple communications protocol: squid writes 
"username password" to the module, the module writes back "OK" or "ERR". For 
group authentication, more information needs to be passed between squid and the 
module. So the ldap_auth acl directive was added to complement the proxy_auth 
directive. 

When communicating with this module when processing the ldap_auth directive, 
squid writes messages of the form (username and the auth-alone flag are 
seperated by a tab ('\t'), all other seperators are spaces):

	username	auth-alone groupcount grouptype #groupname# grouptype \
	   #groupname# ... password

where:
	- username is the user's id, which can contain any character except a 
	  newline ('\n') or tab ('\t')	
	- auth-alone is either 0 or 1, and tells the module to return success if 
	  the username and password are correct, even if the user wasn't a member
	  of any group in the acl
	- groupcount specifies the number of groups that follow 
	- grouptype is either s (for static) or d (for dynamic)
	- groupnames are bracketed by #'s, and may contain spaces
	- password is the users password, and may contain any character except
	  a newline ('\n'), or a hash ('#') followed by a space 

Example:
	
	jack	0 3 s #poor password choosers# s #localusers# d #ou=people# SeCrEt

or

	jack	1 0 SeCrEt 

if there are no groups to be checked. 

The module reads this info, and first verifies the user is in the directory. If 
this fails, a failure flag is returned to squid. If this lookup succeeds, the 
module checks the membership of each group until the user is found or all groups
have been tried. If the user is not found in any of the groups and the auth-alone
flag is 0, a failure flag is returned. 
If the user is found among the groups or the auth-alone flag is 1, the module 
attempts to bind to the directory as that user with the supplied password. If 
that step succeeds, a pass flag is returned to squid along with the group that 
the user was found in (if any). Upon failure, the module returns:
	
	f

upon success, the module returns:
	
	p groupname

or, if the user matched no groups, the auth-alone flag was 1, and the user 
authenticated correctly:

	p


FOR MORE INFO
-------------

Go to http://group-ldap-auth.sourceforge.net/, or email 
tocrawle@users.sourceforge.net.










