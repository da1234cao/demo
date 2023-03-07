#!/usr/bin/env bash

# 执行失败立即停止
set -e 

cp -f build/server /usr/local/bin/
cp -f server.service /etc/systemd/system/
systemctl daemon-reload