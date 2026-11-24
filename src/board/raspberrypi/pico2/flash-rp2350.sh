#!/usr/bin/env bash
set -e
probe-rs download --chip RP235x "$1"
probe-rs reset --chip RP235x
