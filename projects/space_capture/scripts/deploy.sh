#!/usr/bin/env bash

set -e

BIN_DIR=/usr/bin/deploy
ETC_DIR=/etc/deploy
OZIFI_ROOT=/sources/production/ozifi
OZIFI_BUILD_ROOT=/sources/production/ozifi-build
APP_NAME=space_capture_server
CONFIG_NAME=space-capture-server.ini
SERVICE_NAME=space-capture-server

cd $OZIFI_ROOT
git pull
cd $OZIFI_BUILD_ROOT
cmake $OZIFI_ROOT
make
rm -f $BIN_DIR/$APP_NAME
cp $OZIFI_BUILD_ROOT/projects/space_capture/server/$APP_NAME $BIN_DIR
cp $OZIFI_ROOT/projects/space_capture/configs/$CONFIG_NAME $ETC_DIR
sudo service $SERVICE_NAME restart

