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
DIST_SUBDIRS = . test ptest
.PHONY: FORCE
# bitcoin core #
DACRS_CORE_H = \
  addrman.h \
  alert.h \
  allocators.h \
  base58.h bignum.h \
  bloom.h \
  chainparams.h \
  checkpoints.h \
  checkqueue.h \
  clientversion.h \
  compat\compat.h \
  core.h \
  database.h \
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
  random.h   \
  rpc/rpcclient.h \
  rpc/rpcprotocol.h \
  rpc/rpcserver.h \
  support/cleanse.h \
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
  wallet/wallet.h \
  wallet/db.h  \
  arith_uint256.h 

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
  
VMLUA_H = \
	vm/lua/lapi.h \
	vm/lua/lauxlib.h \
	vm/lua/lcode.h \
	vm/lua/lctype.h \
	vm/lua/ldebug.h \
	vm/lua/ldo.h \
	vm/lua/lfunc.h \
	vm/lua/lgc.h \
    vm/lua/llex.h \
	vm/lua/llimits.h \
	vm/lua/lmem.h \
	vm/lua/lobject.h \
	vm/lua/lopcodes.h \
	vm/lua/lparser.h \
	vm/lua/lprefix.h \
	vm/lua/lstate.h \
	vm/lua/lstring.h \
	vm/lua/ltable.h \
	vm/lua/ltm.h \
	vm/lua/lua.h \
	vm/lua/lua.hpp \
	vm/lua/luaconf.h \
	vm/lua/lualib.h \
	vm/lua/lundump.h \
	vm/lua/lvm.h \
	vm/lua/lzio.h 	
  
VM_H = \
	vm/vm8051.h \
	vm/vmrunevn.h \
	vm/script.h \
	vm/testmcu.h \
	vm/appaccount.h \
	vm/vmlua.h

	
VM_CPP = \
	vm/vm8051.cpp \
	vm/vmrunevn.cpp \
	vm/script.cpp \
	vm/testmcu.cpp \
	vm/appaccount.cpp \
	vm/lmylib.cpp \
	vm/vmlua.cpp 
	 

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
  database.cpp \
  init.cpp \
  leveldbwrapper.cpp \
  main.cpp \
  miner.cpp \
  net.cpp \
  noui.cpp \
  rpc/rpcblockchain.cpp \
  rpc/rpcmining.cpp \
  rpc/rpcmisc.cpp \
  rpc/rpcnet.cpp \
  rpc/rpcrawtransaction.cpp \
  rpc/rpcserver.cpp \
  txdb.cpp \
  txmempool.cpp \
  cuiserver.cpp	\
  $(VMLUA_H) \
  $(VM_CPP) \
  $(VM_H) \
  $(JSON_H) \
  $(DACRS_CORE_H)

libdacrs_wallet_a_SOURCES = \
  wallet/db.cpp  \
  crypter.cpp \
  rpc/rpcdump.cpp \
  rpc/rpcwallet.cpp \
  keystore.cpp \ 
  wallet/wallet.cpp \
  wallet/walletdb.cpp \
  rpc/rpctx.cpp \
  $(DACRS_CORE_H)

libdacrs_common_a_SOURCES = \
  base58.cpp \
  allocators.cpp \
  arith_uint256.cpp \
  chainparams.cpp \
  core.cpp \
  hash.cpp \
  key.cpp \
  netbase.cpp \
  protocol.cpp \
  random.cpp  \
  rpc/rpcprotocol.cpp \
  support/cleanse.cpp \
  sync.cpp \
  syncdata.cpp \
  syncdatadb.cpp \
  util.cpp \
  uint256.cpp \
  version.cpp \
  json/json_spirit_reader.cpp \
  json/json_spirit_value.cpp \
  json/json_spirit_writer.cpp \
  $(DACRS_CORE_H)

if GLIBC_BACK_COMPAT
libdacrs_common_a_SOURCES += compat/glibc_compat.cpp
libdacrs_common_a_SOURCES += compat/glibcxx_compat.cpp
endif

libdacrs_cli_a_SOURCES = \
  rpc/rpcclient.cpp \
  $(DACRS_CORE_H)

nodist_libdacrs_common_a_SOURCES = $(top_srcdir)/src/obj/build.h
#

# bitcoind binary #
dacrs_d_LDADD = \
  libdacrs_server.a \
  libdacrs_wallet.a \
  libdacrs_cli.a \
  libdacrs_common.a \
  liblua53.a \
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
  liblua53.a \
  $(LIBLEVELDB) \
  $(LIBMEMENV) \
  $(BDB_LIBS) \
  $(BOOST_LIBS)
dacrs_cli_SOURCES = dacrs-cli.cpp \
	tx.cpp \
	key.cpp \
	main.cpp \
	database.cpp \
	miner.cpp \
	$(VM_CPP) \
	txmempool.cpp \
	checkpoints.cpp \
	txdb.cpp \
	net.cpp \
	bloom.cpp \
	init.cpp \
	alert.cpp \
	addrman.cpp \
	leveldbwrapper.cpp \
	wallet/wallet.cpp \
	rpc/rpcmining.cpp \
	rpc/rpcserver.cpp \
	wallet/db.cpp \
	cuiserver.cpp \
	noui.cpp \
	wallet/walletdb.cpp \
	crypter.cpp \
	keystore.cpp \
	rpc/rpcblockchain.cpp \
	rpc/rpcmisc.cpp \
	rpc/rpcnet.cpp \
	rpc/rpcrawtransaction.cpp \
	rpc/rpctx.cpp \
	rpc/rpcdump.cpp \
	rpc/rpcwallet.cpp
	
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
