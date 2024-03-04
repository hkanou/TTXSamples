TTXWinSCP -- WinSCP Launcher

Feature:
  Add the WinSCP launch menu to TeraTerm.
  Select the directory after the SCP connection from the submenu.

Description:
  This plug-in uses TeraTerm connection information to start WinSCP.

    winscp.exe [schema://[username[:password@]hostname:port[/path/]]

  - winscp.exe
    Use the value of the parameter WinSCPPath in the INI file.
    If not set, use "C:\Program Files (x86)\WinSCP\WinSCP.exe."
    If the path contains white space, enclose it in ".
  - schema
    Use the value of the parameter WinSCPschema in the INI file.
    Specify scp, sftp, ftp, or ftps.
    If not set, scp will be used.
  - username
    Even if a username is given in a command line option (such as /user=),
    that value will not be used.
    This is because the arguments to WinSCP can be referenced using tools
    such as Process Explorer, which could lead to username leaks.
    Although it is possible to pass the username to WinSCP
    by setting AcceptTheRisk to the parameter WinSCPUserEmbed (default:off) in the INI file,
    this setting is deprecated.
    If AcceptTheRisk is set, use the value of the parameter WinSCPUser in the INI file.
    If a username is given in a command line option (such as /user=), 
    that value takes precedence.
  - password
    As with username, this value will not be used even if a password is provided 
    with a command line option (such as /passwd=),
    as this could lead to password leakage.
    Although it is possible to pass a password to WinSCP by setting AcceptTheRisk
    to the parameter WinSCPPasswordEmbed (default:off) in the INI file,
    this setting is deprecated.
  - host
    Use connected hostname in TeraTerm.
  - port
    Use connected port in TeraTerm.
    If a value is set for the parameter WinSCPPort in the INI file, use that value.
  - path
    The values of parameters Dir1 to Dir20 of the INI file are displayed in the submenu.
    The maximum number of items in a submenu is 20.
    If you want to increase the maximum number, increase MAX_DIR_ITEMS to build TTXWinSCP.c.

  The value of the parameter WinSCPMenuString in the INI file is displayed in the menu title.
  If not set, "WinSCP(&J)" is used.

  If the WinSCP section and DirX are not set in the INI file,
  The WinSCP startup menu does not appear.

Configuration example:
  [WinSCP]
  WinSCPMenuString="WinSCP(&J)"
  WinSCPPath="C:\Program Files (x86)\WinSCP\WinSCP.exe"
  WinSCPSchema=scp
  WinSCPPort=22
  WinSCPUserEmbed=off
  WinSCPUser=
  WinSCPPasswordEmbed=off
  Dir1=/
  Dir2=/home
  Dir3=/var
  Dir4=/var/log
  Dir5=/var/tmp
  Dir6=/opt
  Dir7=/tmp

History:
  Ver 0.1 2024.02.25 beta version
  Ver 0.2 2024.03.03 first release
  Ver 0.3 2024.03.05 Corrected to prioritize connected port number.
