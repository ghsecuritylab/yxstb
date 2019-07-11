Log Using Telnet 24 
|
+-->telnet IP 24
|
+-->loghelp  (show info about modulename and modulelevel and how using)
|
+-->logset moduleName moduleLevel
|   
+-->logout 1
|
+-->logstyle 0  
    "logstyle n (n=0:brief log n=1:huawei log n=2:printf)" 

cat /var/debug.ini
[DEBUG]
style = 2
hwUser = 3
hwOper = 3
hwSafe = 3
hwRun  = 3
rtsp = 3

style = 0 简短的打印
style = 1 华为规范的日志,不利于开发人员定位问题.
style = 2 串口小写printf打印,延时小,便于定位死机日志.
