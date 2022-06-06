# 智慧教室 在线监考系统



## 安装说明

### 前端

安装

```shell
cd ant-design-vue-jeecg
cnpm install
```

启动

``` shell
npm serve run
```

### 管理系统



### 算法服务

```

```

### 数据服务(需要docker)

`docker run`后面的`-v`可以根据自己的磁盘情况选择挂载路径

#### mysql

```shell
docker pull mysql

docker run -d -v /d/cache/docker/mysql/conf:/etc/mysql -v /d/cache/docker/mysql/data:/var/lib/mysql -v /d/cache/docker/mysql/mysql-files:/var/lib/mysql-files/ -p 3306:3306 --name smc_mysql -e MYSQL_ROOT_PASSWORD=hongyaohongyao mysql:latest
```

#### redis

```
docker pull redis
```



#### minio

```shell
docker pull minio/minio

docker run -p 9000:9000 -p 9001:9001 --name smc_minio -e "MINIO_ROOT_USER=hongyaohongyao" -e "MINIO_ROOT_PASSWORD=hongyaohongyao123" -v /d/cache/docker/minio/data:/data -d minio/minio server /data --console-address ":9001"
```



#### 流媒体服务器

```shell
docker pull mugennsou/nginx-http-flv

docker run -d -p 8888:80 -p 1935:1935 --name smc_video mugennsou/nginx-http-flv
```

# 感谢

## 系统开发

- [JEECG BOOT](https://github.com/jeecgboot/jeecg-boot) 
- [Drogon](https://github.com/drogonframework/drogon) 
- [tensorRT_Pro](https://github.com/shouxieai/tensorRT_Pro)   

## 算法

- [InsightFace: 2D and 3D Face Analysis Project](https://github.com/deepinsight/insightface) 
- [AlphaPose](https://github.com/MVIG-SJTU/AlphaPose) 