#!/bin/bash

TOP_DIR=/root/work/PP-CNN/build
CLI_DIR=$TOP_DIR/demo/client
SRV_DIR=$TOP_DIR/demo/server

echo "cmp enc inputs"
rm -f a.txt; for ((i=0; i<784; i++)); do echo enc_inputs-$i.dat >> a.txt; cmp $CLI_DIR/enc_inputs-$i.dat $SRV_DIR/enc_inputs-$i.dat; echo $?>> a.txt; done

echo "cmp params"
cmp $CLI_DIR/params.dat $SRV_DIR/params.dat

echo "cmp pubkey"
cmp $CLI_DIR/pubkey.dat $SRV_DIR/pubkey.dat

echo "cmp relinkey"
cmp $CLI_DIR/relinkey.dat $SRV_DIR/relinkey.dat
