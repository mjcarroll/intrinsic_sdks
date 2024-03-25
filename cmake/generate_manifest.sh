#!/usr/bin/env bash

protoc  --deterministic_output --descriptor_set_in=$1 --encode=$2 < $3 > $4
