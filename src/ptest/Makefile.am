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
  updateaccount_tests.cpp \
  mining_tests.cpp	\
  CDarkAndAnony.cpp	\
  ../test/systestbase.cpp \
  CycleSesureTrade_tests.cpp	\
  CycleTestBase.cpp	\
  CycleTestManger.cpp \
  CycleTestManger.h	\
  CycleTestBase.h	\
  CycleSesureTrade_tests.h	\
  CDarkAndAnony.h	\
  ../test/systestbase.h \
  CycleP2PBet_test.cpp \
  CycleP2PBet_test.h \
  createminterkey.cpp \
  createminterkey.h