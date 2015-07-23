/*
 * CAnony_tests.cpp
 *
 *  Created on: 2015-04-24
 *      Author: frank
 */

#include "CIpo_tests.h"
#include "CycleTestManger.h"
#include <boost/assign/list_of.hpp>
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_reader.h"
using namespace std;
using namespace boost;
using namespace boost::assign;
using namespace json_spirit;

typedef struct user{
	unsigned char address[35];
	int64_t money;
	int64_t freemoney;
	int64_t freeMothmoney;
	user()
	{
		memset(address,0,35);
		money = 0;
		freemoney = 0;
		freeMothmoney = 0;
	}
	IMPLEMENT_SERIALIZE
	(
			for(int i = 0;i < 35;i++)
			READWRITE(address[i]);
			READWRITE(money);
			READWRITE(freemoney);
			READWRITE(freeMothmoney);
	)
}IPO_USER;

typedef struct tagdddd{
	const char * pAddress;
	int64_t nMoney;
}IPO_DATA;

//const int64_t totalSendMoney = 10825192381120926; 二期IPO第一批发币总额
const int64_t totalSendMoney =  2711374031363492; //二期IPO第二批发币总额
IPO_DATA arrayData[]=
{
		/*==========二期第一批发币==============================
		{"DiNgnLSgpXriHG3F9ja4uRNy6KqWACRHhm",	629064563190821         },
		{"DrpuDyNKxW9ZQdEaGQfqqE8FG4JS1NGvXU",	255316412689406         },
		{"Dd6s3vyjYRWmBHXH3pCt22h5dN8zGiWBEj", 	63588119635711          },
		{"DhWFkgpk4519kCsQWhwaLBtUHpjpxP4LLo",	31453228159541          },
		{"DXV3aQ33uQ9Rnf27twnTGi4VH9rBqbfxZw",	102129533888433         },
		{"Dibg2n2Gi8jurJfuKwcS5BhNcnGWM5zUPS",	31264080451014          },
		{"DjruxAzrthxb8exkzVrWamU2tXFwqqpvqA",	76462835239940          },
		{"DbqtB8vmrYA1agczzgYeHrdRUctnKWthZp",	28323635787582          },
		{"DV8DV9LXhgFVdYKnfoULTQrBRdoi76QwKu",	966515090685614         },
		{"DmZSqPmnngAvVyy1pM81s6S6xy8S8DYXqQ",	15755120414135          },
		{"DphRgxfaz2csPnApy5XRyKHD6AXQVoWAnG",	15301136025278          },
		{"DpGE9ESxY5iwGY6vNdLhxBucqRUJTuBNGp",	4457237253773           },
		{"DUXjstf2ajk7EdSc3fyMTsYrBKGz9BW6RZ",	505169210837967         },
		{"Dcr3EqfxnfPULbUqDN82vzx8dSdDSB5jML",  1279140344135300        },
		{"DVGpJDtfrnP7RwRm9DG2eJe2cA7d4AZehD", 	158059059550680         },
		{"DegEa9XQDNgkcCvjxHj638Q6JvMxWttWXq",	8468022303549           },
		{"DfAHprmHREwii5E5bimY6shJSXCPYBN8jg",	27606877438171          },
		{"Dk6xebdxLfSZqtAeKZT9MTqk6Bgqjgj4eH", 	44404548174746          },
		{"DcbsCnyx3aDCnfhvSYgPFCwhQyDGFER9fs",	306022720505554         },
		{"DazHyHxDzYLdXNYJ2SU9oGxg3jv9pUC4PE",	5723401676532           },
		{"Depa1cRooTp7aiXYV12YDAwHchHXgZtg94",  123314804821065         },
		{"DohWxvnGe4mwpdw5fyNSAjNnbZa1iLanrm",  63247287977396      	},
		{"DiTzL8SH7G4PbD3BS9R7kwnc5EE26U3HTb",	84282706761775          },
		{"Dcso2zWpPjTpWHSUafvSoLRrX1uJ7CuQyH",	543634646257502         },
		{"DsGKryXGAfrP4WFXxc8KWHQm8sa3veBhGA",	46762237974581          },
		{"DadrYMgyhZLxAxpQ445ovggPTN6vbU8GwP",	62349650632775          },
		{"DfJvvv6dwJdM6XjZMveFc87rrmdWWUSrjE",	125812912638164         },
		{"DfUwCz6oc2r69HWMjAKSDZoM3LgWe8qV4J",	304882092016040         },
		{"Dkug9qr97K8PKuadGtfm6HtUDZJQnR3d2c",	15609584690444          },
		{"DfuJGcRJSd7B97xGvCpntr8mWr6igXcDgT",	1067678921525030        },
		{"DmcVGPMFYrwkQGgmQZ9KJYHeg69AyytsL6",	158506386109728         },
		{"DWREGhS13CsW1EmKajSR9N1JmSprhi74nR",	14308504191330          },
		{"Dd2GPY96ppieZbA3navcgTEhNv9ibHkwPS",	47044881983508          },
		{"DpLXtNfYaqgNvUxnv9YYHCfMNTfYFZRLkW",	62438338761775          },
		{"DbR4cCe588PGHtqFMgDbPharR7yRcsRbR1",	31936178709025          },
		{"Da3vw89ifpJCAKFGhwKg3HsEZjX6jJuRer",	7942099106461           },
		{"Dibg2n2Gi8jurJfuKwcS5BhNcnGWM5zUPS",	99689024416847          },
		{"DVM9nkYGZ1VfBZqerZFCSipXbLv2oNxQ63",	4676223797458           },
		{"DpxNEQ1ny5mpuYvxPhNoJnBz4oxdoGt3fS",	47522063815343          },
		{"DVJV1BCRTKri1bdRBtMFyJyZ8BsokWVuoy",	15609584690444          },
		{"Dj2x6yknXsgMA2twSZ2EV7PdANM3v4VmP2",	713147540713311         },
		{"DhHhAgsb4ihDZBmEe97TeyvbpJqmKj2t9c",	14060737911233          },
		{"DeSpoykJvCFWDSNoPypPSsKQtyvLZgopTy",	31687149536050          },
		{"DZof2fTwS4NVmagv6tvEiEEZvL5ZJPdDM6",	6336275175379           },
		{"DkQmnoJ3Qod8LiNiqus5nCFQbfDBnpTZfz",	27624239511264          },
		{"DkZwdzgdE7yNkwyXVENhaAWE1r1A1d4KiT",	11982224172930          },
		{"Djqfx3kEzRybBdpD1gQynpmZkBNNxpaEyX", 	451063463366721         },
		{"DmGDKuzDnXgqGQArkEV3nKThuwLRZkrFjc",	46200000000000          },
		{"DqDgSEst3xGvjmoVnKo2Bj9rnj4etEXqr8",	92353800000000          },
		{"DeY7XZhs8FSF1joLxMt2sEHxaTfAEupcc6",	13860000000000          },
		{"DnM7bcHHpkRa1DuSdrF3JFfUFssrVmjgAi",	46200000000000          },
		{"DYErfr68zMPw4A2bR7zzMyc9UZKuZxu5fg", 	91560000000000          },
		{"DnCAcDosTcs4d7b8X9xAPN3AjahWHH7tN9",	68078102400000          },
		{"DknzqLdjCfLtmJyBhtUr6iDG4Rj2U2EeP9",	5957060037600           },
		{"Deu6Qa9rXKdSDfVTrX4MtcW8zHeM4ftw6U",	199200288000000         },
		{"Dih7d4fmSbMmAnhrya9iKFTVoVLukyXfk1",	51643200000000          },
		{"DmNSTMgDdtkpAonUoQSnrVT9CZGFjKFdu7",	44520000000000          },
		{"DneUvGLqNjAtoiMN2ZNaKTgjvS9AqWZW3c",	18952164000000          },
		{"DWYEL2FWpRThZayXeZgzZazcx6PJCrRLV3",	93046800000000          },
		{"DjaiXmw7YGLzBm5SZqVyhKvEVLg9vW2hdG",	46786068000000          },
		{"DZRLa31S7bfB9cyFpk3s7k72XXQ39ExiNy",	4452000000000           },
		{"DeRxv9Jvg5EQd1Ra88StC2EGpkJACjtgod", 	13356000000000          },
		{"DsmKrSuoEsRrbig7SAWy7Xkdv2zkxzLEcp",	240240000000000         },
		{"DVYqE9rw1nCAHFyMDnxYCNEjePvGYviizH", 	221710756176000         },
		{"Dg9YNrSxsXnvaB6kY1EACVEY172TbFtPTj",	21840000000000          },
		{"DicZ2eRRqNZSJ6NyQev4BP1p6n5LEcJf7v",	727765584000000         },
		{"DkQ2qRMnJByiEedLYhsnobKM1EaQ5HQ1rD", 	21881059200000          },
		{"DUV1Dz8UvQ9wtwPLynxBzuL8RY9qZ1zuWL",	8211840000000           },
		{"DWz5DGp2vmt27ZHLVcNFs1ENV8fA1GCEc9",	10002720000000          }
		*/
		/*=================二期第二批发币=======================================*/
		{"DqPeihPeRu5pfkQe7Fh7BqiPvNFnAa8kUo",	152441046008020  },
		{"DneUvGLqNjAtoiMN2ZNaKTgjvS9AqWZW3c",	6426978295234    },
		{"Dnv9t7LjjTnGaXVKCx5JXaCCWiB5uwwpfe",	69917167495968   },
		{"DZC4ciiHGCu61DcT4t9Qvo9caiDtJVf5q6",	228909351311202  },
		{"DojtnG5yZSxPRSEvPKavURkwRbn1Zn3uuU",	15389547932030   },
		{"DbehphkhfrVrFAiVG5ceh5VBRDAuJjUMqo",	29955560432326   },
		{"DsHiFtvJegzQzGytsnaNWDD6Ezh5MYQ3kV", 	159892543016912  },
		{"DgRfP5eRZJ4c1NTfdSgauaZqSwx6JFGnqn",	319785086033825  },
		{"DsVVWG8u5BJCZjFsDhx4y2MXs1NTWhhm8Y",	9435968447862    },
		{"DpNAiXKjtopk4x7dLeqfKyUiGHY8Spm2zj",	311600018884850  },
		{"DmzZcfxVHA6VLRprNYqZzMgDACy5ZaVFk5",	31174825316387   },
		{"DpjZdd9n9mMbGip5iyC6vRCbTVZpAYSyLq",	9672226111741    },
		{"Dk3BxGcXpLKcbEjZo1B7thMyjPHEhZA5Ae",	82872718533790   },
		{"DrN1ffcLwrbAnCWoP9ePMx7Ko6WB4jQR4E",	31729961543345   },
		{"DbJCJvvtwhRpEGdhF2iwDuj1VamtyyK8bD",	138600000000000  },
		{"DVxpGL4dUqhLzrJmVwxbRDEFSwCKiBA9bX",	28291032000000   },
		{"Do611mHxmLmiLJyrQM6NLBPqQsYtxjBUiR",	907200000000000  },
		{"Dg7iqYXABJu53ymffxKgc1Bk4K9W1UonJ6",  178080000000000  }


		/*测试发币
		{"e1pzvqWNDezm3DNqoTZgVEWZ4avBgWe2c5",	629064563190821         },
		{"e1dEuSNrgiURqL61qEUKqtbrh4SS54ow39",	255316412689406         },
		{"dmwLekTvkdmjKKNsUJNmUGXq4KzUQvWNhM", 	63588119635711          },
		{"du39jqpNvbxaUDjFbWNqqydGjtLtZiqByq",	31453228159541          },
		{"dhqJ5QXyRwmCSbw1Gj9jCQwSg2xPcrKZQr",	102129533888433         },
		{"duFcCMuXtELJVYzbqRXZ2RCbN3vBmytFe9",	31264080451014          },
		{"dnyUwCZT1nhmQH3fa8GZig8LRmkBYT48m4",	76462835239940          },
		{"dePxY7knqTU21tKXYF5gLBJnmNMb8e9H2r",	28323635787582          },
		{"dtB5EDtydo4Yvmaon5HpwjkBYfsV2tbTN2",	966515090685614         },
		{"dfuqbuXmsNWrY5cayKMb4qVKn8EZNMWfrf",	15755120414135          },
		{"dzVc4TdXJBdXLF9yTsEg7zVEKYTmQ16u3n",	15301136025278          },
		{"drdxMCY7mFyCrKw58VpwGadVC4TiL85jhG",	4457237253773           },
		{"df9ryp7TPchvyzKGAi3Nj87KYeUQfnHahc",	505169210837967         },
		{"dq4PhbcFTqijXKc8SF5FpZR7vMTeXHASXG",  1279140344135300        },
		{"dwygx8bexsat8Pqs8T17pc99khPvMLMJjj", 	158059059550680         },
		{"dr13BprMnhBuqwwePo4LEzHE1CxKDShbKG",	8468022303549           },
		{"do22zke78bz3F49MAGhQk8jFUmXqBfpbip",	27606877438171          },
		{"dg7gANKTD6sCnpopPcbiU51m15WQmihiua", 	44404548174746          },
		{"ds31UhuB3a5KmhNcLBFrwtJpvSpKchHsmm",	306022720505554         },
		{"djZU3bKwggaAJYuwJdkfDZjV7y5to22x3z",	5723401676532           },
		{"duqKKByNNGSEzfJTJjP3hv8SEAgBikRLyD",  123314804821065         },
		{"dkqLckzJaTqgT1KJ5hVBRG8ZB1TBPanb49",  63247287977396      	},
		{"djW171pyq1e9odLB1pPRJEJEDohRk6LyEn",	84282706761775          },
		{"dkTwTAHEPNPowTvna88ZKoffJH3eR2vzQz",	543634646257502         },
		{"de3qRpoP7jogfPa1WRpFXNESJjNWoMgHHu",	46762237974581          },
		{"dr2XYMzABBqgcbmACw5Uigac8DnpXSatnE",	62349650632775          },
		{"doSsredaG4Levipfn5uGTVvmy9PeHMtGke",	125812912638164         },
		{"dkhz6w3AvD1K1Yxef6LaRFFCx6HcuCC2tj",	304882092016040         },
		{"dfATNBRXRW7VJT7uat6bsoHjf4e2BMMCCx",	15609584690444          },
		{"diX2e4W7WAssivLk95VYkhJQoa6o7TLjAb",	1067678921525030        },
		{"duDJypK1ovBsEJrBgXiK6gcKFyKgh79pRw",	158506386109728         },
		{"djFomBLzyv1n5vbkqukmA4AJcwYy19KwvG",	14308504191330          },
		{"dwgmLAdB6QtUrubCoK6EY6Qae1JZS46bN6",	47044881983508          },
		{"detarHmZdYgjwgUATBQezjorTVjz1Fg2ct",	62438338761775          },
		{"dza6xDF4gow9HuyMZGTCZRuWrLmggu9sc9",	31936178709025          },
		{"dk3N7urDQyUyn7ipYdLwUNFh2Q6vMRk8VT",	7942099106461           },
		{"dgf84RRrzHknDMHcZeLKRKQCBZpwc8aNhF",	99689024416847          },
		{"dnspcVrPZBRyryQUVv7FopfnxSWMfh6iQ1",	4676223797458           },
		{"du7i5qX5sF6obSPPKmFTrEiqqWeEoSRbWD",	47522063815343          },
		{"de3nGsPR6i9qpQTNnpC9ASMpFKbKzzFLYF",	15609584690444          },
		{"djY7nHA1JY8vZxsU4Q2WAamcpgB7nVrCN6",	713147540713311         },
		{"dwxNRR4uRQwAt7KpWqvJqCM7Q7rYqhRB6r",	14060737911233          },
		{"dtArFb8L6d9ZySxKSwfHvT4KdbBKxbu8Qb",	31687149536050          },
		{"dsgFSKxXcD49t5ssgjCzvZJh65X36AJcij",	6336275175379           },
		{"dnzgmB8aBrcp5uipdnuHyMRjZPPpYrT8cR",	27624239511264          },
		{"dwX95hc7NrfX1nHf49EDQ3bmbbHuFtHr2d",	11982224172930          },
		{"dk7AJyimGutonDfLqZYFqSFAVzjkhY4uNm", 	451063463366721         },
		{"ddjpzUGRBE3Hipb643qexPxvK2CNTouC5L",	46200000000000          },
		{"dnfAgmPQVktjASJoBfyhK2SKLadDfUCG7e",	92353800000000          },
		{"djEcj9uBFF2dwQzKQo67m9ff2L8FeT6NX8",	13860000000000          },
		{"dnH5wdz8UpizJYBgJ7bfpHxH2cvxLqYHSs",	46200000000000          },
		{"dpWq5apDViHRiyT4EdhddwwVjufQDj3ahF", 	91560000000000          },
		{"dzbJTzwFKh6JyoSFJA73CJA7Ht8137jRHg",	68078102400000          },
		{"dxEkC8u3Sy9d1iVwGDizBnFZwpFjzANvrk",	5957060037600           },
		{"dpepZLbFV153ksvTdgdnEfC4GgfnARG7ab",	199200288000000         },
		{"drAbzQnV7cheogLqk9DBfhXcy24VVPLQQV",	51643200000000          },
		{"deaZqVgVSfG59SV3mdP4rS6oxrWF9U6QvK",	44520000000000          },
		{"dpnTnhF17dexUX15YwkgYjgDWX1vE4D84J",	18952164000000          },
		{"dvvHezmTVGgQuyETFmDnUzccA2xALwQcEJ",	93046800000000          },
		{"dsBo7mpPzDPPkK3op3mJamzWufHaXMPnCF",	46786068000000          },
		{"dyQsvK5n6vJMCDNH4iwzUNiRsajhJGD2UB",	4452000000000           },
		{"diKJsTsv2ttzW2PEHwDyd1sYemBU8p7qq7", 	13356000000000          },
		{"dxVVoz3pngMV1xeib1KDme3AgYHqhk8sAe",	240240000000000         },
		{"de7gwNvGSTq99ojbUp5UCqRVc5stNdo7st", 	221710756176000         },
		{"dxiCFSNNK8GD4i5hP9Gkbs4ufLYKV6meho",	21840000000000          },
		{"duanHTqvNcEukQj9HdaLYP9nYkA4ix9y1m",	727765584000000         },
		{"de6trrHUFwTpd37kHpKhL17CrjRaSYFuXj", 	21881059200000          },
		{"dkb9ySUDik9U2DkoYtELEBTi6wRMtsU1Qj",	8211840000000           },
		{"dztCKNQ7j6JuDS3CydRtB3iL9Cf369fqDS",	10002720000000          } */
};



#define max_user 100


static IPO_USER userarray[max_user];
CIpoTest::CIpoTest():nNum(0), nStep(0), strTxHash(""), strAppRegId("") {

}

TEST_STATE CIpoTest::Run(){

//	int addrcount = 0;
//    ifstream file;
//    string strCurDir ="/home/share/bess/dacrs_test/ipo.txt";
//	file.open(strCurDir, ios::in | ios::ate);
//	if (!file.is_open())
//		throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot open wallet dump file");
//
//	file.seekg(0, file.beg);
//	if (file.good()){
//		Value reply;
//		json_spirit::read(file,reply);
//		const Array & keyarry = reply.get_array();
//		for(auto const &keyItem :keyarry)
//		{
//			string addr = find_value(keyItem.get_obj(), "addr").get_str();
//			memcpy((char*)userarray[addrcount].address,(char*)addr.c_str(),sizeof(userarray[addrcount].address));
//			userarray[addrcount].money  = find_value(keyItem.get_obj(), "money").get_int64();
//			userarray[addrcount].freemoney = find_value(keyItem.get_obj(), "freemoney").get_int64();
//			userarray[addrcount].freeMothmoney = find_value(keyItem.get_obj(), "freeMothmoney").get_int64();
//			addrcount++;
//			if(addrcount == (max_user -1))
//				break;
//		}
//	}
//	file.close();


	for (int i = 0; i < max_user; i++) {
		string newaddr;
		BOOST_CHECK(basetest.GetNewAddr(newaddr, true));
		memcpy((char*)userarray[i].address,(char*)newaddr.c_str(),sizeof(userarray[i].address));
		userarray[i].money = 10000;
		userarray[i].freemoney = 200;
		userarray[i].freeMothmoney = 22;
	}

    // 注册ipo脚本
	RegistScript();

	/// 等待ipo脚本被确认到block中
	while(true)
	{
		if(WaitComfirmed(strTxHash, strAppRegId)) {
					break;
				}
	}
	/// 给每个地址转一定的金额
	int64_t money = COIN;
	for(int i=0;i <max_user;i++)
	{
		string des =strprintf("%s", userarray[i].address);
		basetest.CreateNormalTx(des,money);
	}

	 cout<<"end mempool"<<endl;
	while(true)
	{
		if(basetest.IsMemoryPoolEmpty())
			break;
		MilliSleep(100);
	}

   cout<<"SendIpoTx start"<<endl;
	SendIpoTx();
	 cout<<"SendIpoTx end"<<endl;
}

bool CIpoTest::RegistScript(){

	const char* pKey[] = { "cNcJkU44oG3etbWoEvY46i5qWPeE8jVb7K44keXxEQxsXUZ85MKU",
			"cNcJkU44oG3etbWoEvY46i5qWPeE8jVb7K44keXxEQxsXUZ85MKU"};
	int nCount = sizeof(pKey) / sizeof(char*);
	basetest.ImportWalletKey(pKey, nCount);

	string strFileName("IpoApp.bin");
	int nFee = basetest.GetRandomFee();
	int nCurHight;
	basetest.GetBlockHeight(nCurHight);
	string regAddr="dk2NNjraSvquD9b4SQbysVRQeFikA55HLi";

	//reg anony app
	Value regscript = basetest.RegisterAppTx(regAddr, strFileName, nCurHight, nFee+20*COIN);
	if(basetest.GetHashFromCreatedTx(regscript, strTxHash)){
		return true;
	}
	return false;
}

bool CIpoTest::CreateIpoTx(string contact,int64_t llSendTotal){
	int pre =0xff;
	int type = 2;
	string buffer =strprintf("%02x%02x", pre,type);

	buffer += contact;

	Value  retValue = basetest.CreateContractTx(strAppRegId, SEND_A, buffer, 0, 10*COIN, llSendTotal);
	if(basetest.GetHashFromCreatedTx(retValue, strTxHash)){
			return true;
	}
	return false;
}
bool CIpoTest::SendIpoTx()
{
	for(int i =0;i <max_user;i++)
	{
		CDataStream scriptData(SER_DISK, CLIENT_VERSION);
		scriptData << userarray[i];
		string sendcontract = HexStr(scriptData);
		CreateIpoTx(sendcontract,userarray[i].money);
	}
	return true;
}
BOOST_FIXTURE_TEST_SUITE(CreateIpoTxTest,CIpoTest)

BOOST_FIXTURE_TEST_CASE(Test,CIpoTest)
{
//	while(true)
//	{
//		string newaddr;
//		BOOST_CHECK(basetest.GetNewAddr(newaddr, true));
//		cout<<"len:"<<newaddr.length()<<endl;
//		if(newaddr.length() != 34)
//		{
//			cout<<"address:"<<newaddr.c_str()<<endl;
//			break;
//		}
//	}
	Run();
}

typedef struct _IPOCON{
	unsigned char address[35];
	int64_t money;
}IPO_COIN;
#define max_2ipouser 100

BOOST_FIXTURE_TEST_CASE(get_coin,CIpoTest)
{

	// 创建转账交易并且保存转账交易的hash
	Object objRet;
	Array SucceedArray;
	Array UnSucceedArray;
	ofstream file("ipo_ret", ios::out | ios::ate);
	if (!file.is_open())
		throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot open wallet dump file");

	map<string, string> mapTxHash;
	int64_t nMoneySend(0);
	size_t t_num = sizeof(arrayData) / sizeof(arrayData[0]);
	for (size_t i = 0; i <t_num ; ++i) {
		nMoneySend += arrayData[i].nMoney;
	}
	BOOST_CHECK(nMoneySend == totalSendMoney);
	for (size_t i = 0; i < t_num; ++i) {
		string des = strprintf("%s", arrayData[i].pAddress);
		int64_t nMoney = arrayData[i].nMoney;
		Value ret = basetest.CreateNormalTx(des, nMoney);
		string txHash;
		Object obj;
		if(basetest.GetHashFromCreatedTx(ret, txHash)) {
			mapTxHash[des]= txHash;
			obj.push_back(Pair("addr", des));
			obj.push_back(Pair("amount", nMoney));
			obj.push_back(Pair("txhash", txHash));
			SucceedArray.push_back(obj);
		} else {
			obj.push_back(Pair("addr", des));
			obj.push_back(Pair("amount", nMoney));
			UnSucceedArray.push_back(obj);
		}
	}
	objRet.push_back(Pair("succeed", SucceedArray));
	objRet.push_back(Pair("unsucceed", UnSucceedArray));
	file << json_spirit::write_string(Value(objRet), true).c_str();
	file.close();

	//确保每个转账交易被确认在block中才退出
	while(mapTxHash.size() != 0)
	{
		map<string, string>::iterator it = mapTxHash.begin();
		for(;it != mapTxHash.end();){
			string addr = it->first;
			string hash = it->second;
			string regindex = "";
			if(basetest.GetTxConfirmedRegID(hash,regindex)){
				it = mapTxHash.erase(it);
			}else{
				it++;
			}
		}
		MilliSleep(100);
	}

	for (size_t i = 0; i < t_num; ++i) {
		uint64_t acctValue = basetest.GetBalance(arrayData[i].pAddress);
		BOOST_CHECK(acctValue >= (uint64_t)arrayData[i].nMoney);
	}
}
BOOST_FIXTURE_TEST_CASE(check_coin,CIpoTest)
{
	size_t t_num = sizeof(arrayData) / sizeof(arrayData[0]);
	for (size_t i = 0; i < t_num; ++i) {
		uint64_t acctValue = basetest.GetBalance(arrayData[i].pAddress);
		string errorMsg = strprintf("acctValue = %lld, realValue= %lld, address=%s \n",acctValue,  arrayData[i].nMoney, arrayData[i].pAddress);
		BOOST_CHECK_MESSAGE(acctValue >= (uint64_t )arrayData[i].nMoney, errorMsg);
	}
}
BOOST_AUTO_TEST_SUITE_END()

