# virtu

# System Labs – Docker, Linux, Kernel

This repository contains all practical work related to Docker, Linux internals (processes, memory, IPC, inodes), security, and kernel module development.

## 1. Docker

Includes:

* Basic Docker usage
* Custom Dockerfile
  * installs gcc
  * creates `/home/script`
  * copies `script.sh`
  * sets it as entrypoint
* Local Docker registry with HTTPS
* docker-compose examples

## 2. Linux

Demonstrations and C programs:

* Process creation and zombie handling
* Custom signal handlers
* Shared memory IPC
* Controlled buffer overflow
* Inode experiments
  * fake JPG + block calculation
  * inode saturation using tiny files

## 3. Kernel development

Kernel modules:

* Hello World module
* Crash module (intentional Oops via array overflow)
* Experiments with kernel panics
* Makefile for building against `linux-headers-$(uname -r)`

## 4. Docker security

Includes:

* Privilege escalation demo with SUID binary in mounted volume
* Explanation of attack scenario
* Mitigation strategies
