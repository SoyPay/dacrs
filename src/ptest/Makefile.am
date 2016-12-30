include $(top_srcdir)/src/Makefile.include

AM_CPPFLAGS += -I$(top_srcdir)/src

bin_PROGRAMS = dacrs_ptest

TESTS = test

# test_bitcoin binary #
dacrs_ptest_CPPFLAGS = $(AM_CPPFLAGS) $(TESTDEFS)
dacrs_ptest_LDADD = $(LIBBITCOIN_SERVER) \
               $(LIBBITCOIN_WALLET)   \
			   $(LIBBITCOIN_CLI) \
			   $(LIBBITCOIN_COMMON) \
			   $(LIBLUA53) \
			   $(LIBLEVELDB) \
			   $(LIBMEMENV) \
               $(BOOST_LIBS) \
			   $(BOOST_UNIT_TEST_FRAMEWORK_LIB)
#if ENABLE_WALLET
#dacrs_ptest_LDADD += $(LIBBITCOIN_WALLET)
#endif
dacrs_ptest_LDADD += $(BDB_LIBS)

dacrs_ptest_SOURCES = \
  p_test.cpp \
  main_tests.cpp \
  UpdateAccount_tests.cpp \
  mining_tests.cpp	\
  create_tx_tests.cpp \
  black_halo_tests.cpp	\
  anony_tests.cpp     \
  ../test/systestbase.cpp \
  cycle_sesure_trade_tests.cpp	\
  cycle_test_base.cpp	\
  cycle_test_manger.cpp \
  cycle_test_manger.h	\
  cycle_test_base.h	\
  cycle_sesure_trade_tests.h	\
  black_halo_tests.h	\
  anony_tests.h   \
  create_tx_tests.h  \
  ../test/systestbase.h \
  cycle_p2p_bet_tests.cpp \
  cycle_p2p_bet_tests.h \
  create_minter_key_tests.cpp \
  create_minter_key_tests.h  \
  ipo_tests.h \
  ipo_tests.cpp \
  red_packet_tests.cpp \
  guarantee_tests.h \
  guarantee_tests.cpp\
  lashou_tests.cpp\
  lashou_tests.h
  
  
  
  
  