# - Find MySQL
# Find the MySQL includes and client library
# This module defines
#  MYSQL_INCLUDE_DIR, where to find mysql.h
#  MYSQL_LIBRARIES, the libraries needed to use MySQL.
#  MYSQL_FOUND, If false, do not try to use MySQL.
#
# Copyright (c) 2006, Jaroslaw Staniek, <js@iidea.pl>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

find_path(MYSQL_INCLUDE_DIR mysql.h
    /usr/include/mysql
    /usr/local/include/mysql
    )

find_library(MYSQL_LIBRARIES NAMES mysqlclient_r
    PATHS
    /usr/lib/mysql
    /usr/lib64/mysql
    /usr/local/lib/mysql
    /usr/local/lib64/mysql
    )

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MYSQL DEFAULT_MSG MYSQL_LIBRARIES MYSQL_INCLUDE_DIR)

mark_as_advanced(MYSQL_INCLUDE_DIR MYSQL_LIBRARIES)
