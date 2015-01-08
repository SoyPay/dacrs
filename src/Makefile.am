include Makefile.include

AM_CPPFLAGS += -I$(builddir)

noinst_LIBRARIES = \
  libdacrs_server.a \
  libdacrs_common.a \
  libdacrs_cli.a
if ENABLE_WALLET
noinst_LIBRARIES += libdacrs_wallet.a
endif

bin_PROGRAMS =

if BUILD_BITCOIND
  bin_PROGRAMS += dacrs-d
endif

if BUILD_BITCOIN_CLI
  bin_PROGRAMS += dacrs-cli
endif

SUBDIRS = . $(BUILD_QT) $(BUILD_P_TEST) $(BUILD_TEST)
#DIST_SUBDIRS = . qt test
DIST_SUBDIRS = . test
.PHONY: FORCE
# bitcoin core #
BITCOIN_CORE_H = \
  addrman.h \
  alert.h \
  allocators.h \
  base58.h bignum.h \
  bloom.h \
  chainparams.h \
  checkpoints.h \
  checkqueue.h \
  clientversion.h \
  compat.h \
  core.h \
  account.h \
  crypter.h \
  hash.h \
  init.h \
  key.h \
  keystore.h \
  leveldbwrapper.h \
  limitedmap.h \
  main.h \
  miner.h \
  mruset.h \
  netbase.h \
  net.h \
  noui.h \
  protocol.h \
  rpcclient.h \
  rpcprotocol.h \
  rpcserver.h \
  serialize.h \
  sync.h \
  threadsafety.h \
  tinyformat.h \
  txdb.h \
  txmempool.h \
  ui_interface.h \
  uint256.h \
  util.h \
  version.h \
  walletdb.h \
  wallet.h

JSON_H = \
  json/json_spirit.h \
  json/json_spirit_error_position.h \
  json/json_spirit_reader.h \
  json/json_spirit_reader_template.h \
  json/json_spirit_stream_reader.h \
  json/json_spirit_utils.h \
  json/json_spirit_value.h \
  json/json_spirit_writer.h \
  json/json_spirit_writer_template.h
  
# VmScript #
VMSCRIPT_H = \
	VmScript/CFundOpeator.h \
	VmScript/CVir8051.h \
	VmScript/CVmRule.h \
	VmScript/VmScript.h \
	VmScript/VmScriptRun.h \
	VmScript/TestMcu.h

VMSCRIPT_CPP = \
	VmScript/CVir8051.cpp \
	VmScript/VmScript.cpp \
	VmScript/VmScriptRun.cpp \
	VmScript/TestMcu.cpp
  

obj/build.h: FORCE
	@$(MKDIR_P) $(abs_top_builddir)/src/obj
	@$(top_srcdir)/share/genbuild.sh $(abs_top_builddir)/src/obj/build.h \
	  $(abs_top_srcdir)
version.o: obj/build.h

libdacrs_server_a_SOURCES = \
  tx.cpp \
  addrman.cpp \
  alert.cpp \
  bloom.cpp \
  checkpoints.cpp \
  account.cpp \
  init.cpp \
  keystore.cpp \
  leveldbwrapper.cpp \
  main.cpp \
  miner.cpp \
  net.cpp \
  noui.cpp \
  rpcblockchain.cpp \
  rpcmining.cpp \
  rpcmisc.cpp \
  rpcnet.cpp \
  rpcrawtransaction.cpp \
  rpcserver.cpp \
  txdb.cpp \
  txmempool.cpp \
  cuiserve.cpp	\
  $(VMSCRIPT_CPP) \
  $(VMSCRIPT_H) \
  $(JSON_H) \
  $(BITCOIN_CORE_H)

libdacrs_wallet_a_SOURCES = \
  crypter.cpp \
  rpcdump.cpp \
  rpcwallet.cpp \
  wallet.cpp \
  walletdb.cpp \
  rpctx.cpp \
  $(BITCOIN_CORE_H)

libdacrs_common_a_SOURCES = \
  base58.cpp \
  allocators.cpp \
  chainparams.cpp \
  core.cpp \
  hash.cpp \
  key.cpp \
  netbase.cpp \
  protocol.cpp \
  rpcprotocol.cpp \
  sync.cpp \
  util.cpp \
  version.cpp \
  json/json_spirit_reader.cpp \
  json/json_spirit_value.cpp \
  json/json_spirit_writer.cpp \
  $(BITCOIN_CORE_H)

if GLIBC_BACK_COMPAT
libdacrs_common_a_SOURCES += compat/glibc_compat.cpp
libdacrs_common_a_SOURCES += compat/glibcxx_compat.cpp
endif

libdacrs_cli_a_SOURCES = \
  rpcclient.cpp \
  $(BITCOIN_CORE_H)

nodist_libdacrs_common_a_SOURCES = $(top_srcdir)/src/obj/build.h
#

# bitcoind binary #
dacrs_d_LDADD = \
  libdacrs_server.a \
  libdacrs_wallet.a \
  libdacrs_cli.a \
  libdacrs_common.a \
  $(LIBLEVELDB) \
  $(LIBMEMENV)
#if ENABLE_WALLET
#dacrs_d_LDADD += libdacrs_wallet.a
#endif
dacrs_d_SOURCES = dacrsd.cpp
#

if TARGET_WINDOWS
dacrs_d_SOURCES += dacrsd-res.rc
endif

AM_CPPFLAGS += $(BDB_CPPFLAGS)
dacrs_d_LDADD += $(BOOST_LIBS) $(BDB_LIBS)

# bitcoin-cli binary #
dacrs_cli_LDADD = \
  libdacrs_cli.a \
  libdacrs_common.a \
  $(BOOST_LIBS)
dacrs_cli_SOURCES = dacrsd-cli.cpp
#

if TARGET_WINDOWS
dacrs_cli_SOURCES += dacrs-cli-res.rc
endif

# NOTE: This dependency is not strictly necessary, but without it make may try to build both in parallel, which breaks the LevelDB build system in a race
leveldb/libleveldb.a: leveldb/libmemenv.a

leveldb/%.a:
	@echo "Building LevelDB ..." && $(MAKE) -C $(@D) $(@F) CXX="$(CXX)" \
	  CC="$(CC)" PLATFORM=$(TARGET_OS) AR="$(AR)" $(LEVELDB_TARGET_FLAGS) \
	  OPT="$(CXXFLAGS) $(CPPFLAGS)"

qt/bitcoinstrings.cpp: $(libdacrs_server_a_SOURCES) $(libdacrs_common_a_SOURCES) $(libdacrs_cli_a_SOURCES)
	@test -n $(XGETTEXT) || echo "xgettext is required for updating translations"
	@cd $(top_srcdir); XGETTEXT=$(XGETTEXT) share/qt/extract_strings_qt.py

#
#CLEANFILES = leveldb/libleveldb.a leveldb/libmemenv.a *.gcda *.gcno
CLEANFILES =  *.gcda *.gcno
#

DISTCLEANFILES = obj/build.h

EXTRA_DIST = leveldb Makefile.include

clean-local:
	-$(MAKE) -C leveldb clean
	rm -f leveldb/*/*.gcno leveldb/helpers/memenv/*.gcno
