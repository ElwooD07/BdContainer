EXT_ROOT = $$PWD/../

GTEST_INC = $$absolute_path($$EXT_ROOT/GTest/src/)
GTEST_LIB = $$absolute_path($$EXT_ROOT/GTest/lib/$$CONFIGNAME)

SQLITE_INC = $$absolute_path($$EXT_ROOT/SQLite/src/)
SQLITE_LIB = $$absolute_path($$EXT_ROOT/SQLite/lib/$$CONFIGNAME)

OPENSSL_INC = $$absolute_path($$EXT_ROOT/openssl-1.0.2l/include64)
OPENSSL_LIB = $$absolute_path($$EXT_ROOT/openssl-1.0.2l/lib64/$$CONFIGNAME)
