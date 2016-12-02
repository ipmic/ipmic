#!/bin/bash

gcc -o select_client select_client.c ../transfer_common.c
gcc -o select_server select_server.c ../transfer_common.c
