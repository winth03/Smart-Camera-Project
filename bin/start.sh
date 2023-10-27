#!/bin/bash
cd "$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")/.."
. ./settings.conf
export PY_PATH=$(pwd)/$python_path
export VENV_PATH=$(pwd)/$venv_path
docker container start $mqtt_docker_name
node-red --settings $node_red_path/settings.js $node_red_path/NodeRedCamera.json
