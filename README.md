流量数据查询服务

需使用 gcc-4.8.1 编译运行

编译 
1.进入src目录
2.make

运行
1.进入script目录
2.sh startup.sh start

结构
1.include 项目头文件  
2.src 项目源码  
3.common 可复用头文件库  
4.util 工具库  
5.third-party 第三方库头文件  
6.lib 项目依赖库  
7.conf 项目配置文件  
8.test 单元测试  
9.script 项目脚本 (monitor脚本放入crontab 可监控服务,并自动重启)  
10.log 日志路径  
11.data 索引数据目录
