Mail-Guide
==========

*Guide zum Setup und Nutzen von Email per Skript unter Verwendung von
mail aus dem Paket mailutils*


**Installation**

- sudo apt-get install mailutils
- sudo apt-get install ssmtp

**Setup**

- sudo vim /etc/ssmtp/ssmtp.conf
- Einkommentieren von FromLineOverride=YES
- Hinzufügen von:

AuthUser=<user>@gmail.com
AuthPass=Your-Gmail-Password
mailhub=smtp.gmail.com:587
UseSTARTTLS=YES
(GMail geeignet ersetzen)

**Anwendungsbeispiele**

$ mail -s "This is the subject" somebody@example.com <<< 'This is the message'
oder
$ echo "This is the body" | mail -s "Subject" -aFrom:Harry\<harry@gmail.com\> someone@example.com


**Programmaufrufe mit Python-Skript**

>>>import subprocess
>>>subprocess.call([<arg1>, <arg2>, ..., <argn>])

<argn> ::== "..."