## Process this file with automake to produce Makefile.in

GIT_VERSION = $(shell sh -c 'git describe --abbrev=4 --dirty --always')

ACLOCAL_AMFLAGS = -I ../m4

ZNCBOTLIBRARY_VERSION=0:0:0

lib_LTLIBRARIES = libzncbot.la

libzncbot_la_SOURCES = \
        zncbot.cpp

libzncbot_la_LDFLAGS = --version-info $(ZNCBOTLIBRARY_VERSION)
