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

//const int64_t totalSendMoney = 10825192381120926; //二期IPO第一批发币总额
//const int64_t totalSendMoney =  2711374031363492; //二期IPO第二批发币总额
//const int64_t totalSendMoney =  2677621584404177; //二期IPO第三批发币总额
//const int64_t totalSendMoney = 19164400000000000;   //一期IPO第一批总额
//const int64_t totalSendMoney =    4913960000000000;   //一期IPO第二批总额
//const int64_t totalSendMoney =    17567366666666608;     /// 一期IPO11个月冻结金额的钱
//const int64_t totalSendMoney =    20000000000000000;     /// 官方冻结金额总额
const int64_t totalSendMoney = 16696666666666610;   //一期IPO10个月冻结金额的钱
IPO_DATA arrayData[]=
{

#if 0
		/*=================一期IPO发币地址一批==============================*/
        { "Do9W61KiBCrtJpQZDELtbrJYVbcuuYhXDv",       305000000000000	},
		{ "DrumFsuGwFzmhDRGgYrzPh4i7sHiSp3iJE",      1260000000000000 },
		{ "DnKUZMvwXfprFCKhnsWRsbJTNnRZg88T2F",      1260000000000000 },
		{ "DftLSeJrMjJJ3UPeehNgArhcoAuDN5422E",      4250000000000000 },
		{ "Dg2dq98hcm84po3RX354SzVyE6DLpxq3QR",      2730000000000000 },
		{ "DnGm7rGyeWchyVJS3VF1mgNyQRjgutphDN",       244000000000000	},
		{ "DXL9ZKSGf3KzJh75gqzb6svbzcyiXb9VNA",       122000000000000	},
		{ "Dpjs5pvXmZbVt3uDEfBrMNbCsWjJzjm8XA",       134200000000000	},
		{ "DZYDEn8CZuwgJ6YS6Zm7VvKaFc6E6tGstz",       134200000000000	},
		{ "DhhGix9J3n8dDvN3zcDi8DTJBSqi1nEe7Z",       600000000000000	},
		{ "DghcznoUhcP8dfZoqT7xoBXw7cbcC72bVw",       720000000000000	},
		{ "DcyumTafQsSh4hJo4V6DaS23Dd2QnpMXKH",      1940000000000000 },
		{ "DhFhqaH8wcb5TSsA4DYEYv4SiKTA4smn1T",       480000000000000	},
		{ "Dg7iqYXABJu53ymffxKgc1Bk4K9W1UonJ6",       71040000000000	}

		/*=================一期IPO发币地址二批==============================*/
		 { "DmznNupXTsmr96w5PTwjx4rGkfXq5u9Gs7",       61000000000000	},
		  { "DZKGAkCpT7rofuz7gWgYS1McNnqajUZJqv",       366000000000000	},
		  { "DYy8MCFATjamvwEUrfun572entvE7hkrGy",      1686000000000000 },
		  { "DrdA3g2edBY6iLR2FxN2nKoPPhgKD9gYgs",       122000000000000	},
		  { "DrFEC23FSRYVLKUKCzDh5i5BLcWD1FPtUy",       1454000000000000},
		  { "DVmVcFtVMRBKuv1JDJVbKQLt1TH8kEtFtW",       239880000000000	},
		  { "DVtwoZaHtcsyxuphPiG3saazLo9sr7QCeK",       240000000000000	},
		  { "DgvRvQHLuPtbPtEcR8Am7RicmsqhRpZ8xt",       240000000000000	},
		  { "Dfkhj8sFCEr4FGPHwj7Qyf3aTQqmxJNW2q",       120000000000000	},
		  { "DhgrzBsyhWaLj4TAvdddsqmyg5W8ABcdM5",       385080000000000	}
#endif
#if 0
	   /*===================一期IPO月冻结金额=============================*/
		{ "Do9W61KiBCrtJpQZDELtbrJYVbcuuYhXDv",       25416666666666	},
		{ "DrumFsuGwFzmhDRGgYrzPh4i7sHiSp3iJE",      105000000000000 },
		{ "DnKUZMvwXfprFCKhnsWRsbJTNnRZg88T2F",      105000000000000 },
		{ "DftLSeJrMjJJ3UPeehNgArhcoAuDN5422E",      354166666666666 },
		{ "Dg2dq98hcm84po3RX354SzVyE6DLpxq3QR",      227500000000000 },
		{ "DnGm7rGyeWchyVJS3VF1mgNyQRjgutphDN",       20333333333333	},
		{ "DXL9ZKSGf3KzJh75gqzb6svbzcyiXb9VNA",       10166666666666	},
		{ "Dpjs5pvXmZbVt3uDEfBrMNbCsWjJzjm8XA",       11183333333333	},
		{ "DZYDEn8CZuwgJ6YS6Zm7VvKaFc6E6tGstz",       11183333333333	},
		{ "DhhGix9J3n8dDvN3zcDi8DTJBSqi1nEe7Z",       50000000000000	},
		{ "DghcznoUhcP8dfZoqT7xoBXw7cbcC72bVw",       60000000000000	},
		{ "DcyumTafQsSh4hJo4V6DaS23Dd2QnpMXKH",      161666666666666 },
		{ "DhFhqaH8wcb5TSsA4DYEYv4SiKTA4smn1T",       40000000000000	},
		{ "Dg7iqYXABJu53ymffxKgc1Bk4K9W1UonJ6",       5920000000000	},

	  { "DmznNupXTsmr96w5PTwjx4rGkfXq5u9Gs7",       5083333333333	},
	  { "DZKGAkCpT7rofuz7gWgYS1McNnqajUZJqv",       30500000000000	},
	  { "DYy8MCFATjamvwEUrfun572entvE7hkrGy",      140500000000000 },
	  { "DrdA3g2edBY6iLR2FxN2nKoPPhgKD9gYgs",       10166666666666	},
	  { "DrFEC23FSRYVLKUKCzDh5i5BLcWD1FPtUy",       121166666666666},
	  { "DVmVcFtVMRBKuv1JDJVbKQLt1TH8kEtFtW",       19990000000000	},
	  { "DVtwoZaHtcsyxuphPiG3saazLo9sr7QCeK",       20000000000000	},
	  { "DgvRvQHLuPtbPtEcR8Am7RicmsqhRpZ8xt",       20000000000000	},
	  { "Dfkhj8sFCEr4FGPHwj7Qyf3aTQqmxJNW2q",       10000000000000	},
	  { "DhgrzBsyhWaLj4TAvdddsqmyg5W8ABcdM5",       32090000000000	}
#endif

//		{"DpqbJLPkBrevxYKpZYAbQp6DJSHzHFiruf",	10000000000000000}, //
//		{"DsSyKYzYBSgyEggq8o6SVD4DnPzETVbaUe",	10000000000000000},
//		{"DmtzzT99HYUGAV6ejkWTWXF8pcYXtkpU4g", -1517878787878782}
		{"DpqbJLPkBrevxYKpZYAbQp6DJSHzHFiruf", 10000}

#if 0  //测试网络 25个测试地址 不要删了
		{"dyjC8fuSoVGpepRGi8F2SridVX4VjykLG4",	100000000}, //
		{"dfLo3CHErzWPrxRtthMiitcoGkrR6DskiF",	100000001},
		{"dwpwbNeGEP9bZzFfNnn4gK96LKNA83R93D",	105000000000000},
		{"e1eHyMk4D9oeUnKsiDjS8KLkpGVRAbPfW6",	354166666666666},
		{"diANoyDdGjvdsYxjrSpcVW4kFSkdEj21PD",	227500000000000},
		{"dq6uutNhgA4y7eUQzZ6kg7NQ4NjzCqnBxa",	20333333333333},
		{"dq4Y9XAMDH8tGKZ5vVeYBgMgRS82yuz9qK",	10166666666666},
		{"dqvbTqp5jhLbdHKUERRdzwdnNJSv43boSq",	11183333333333}, //
		{"diNzPQ7z9UDcrWXngsB8eGFJCa58rsb37N",	11183333333333},
		{"diMKZ6cBrcxqnSiXsDEgdPawQPbTdWBEdj",	50000000000000},

		{"dfraKJQTwBSHNhLSQqtdfNAf5xdwK5PBK5",	60000000000000},
		{"dgVST4WVoovPv9QFJAQXbK2c6EiGG4Xctd",	161666666666666},
		{"dp2KppVD9zs1kjv3NFp3U5kJEfH6ySE1nQ",	40000000000000},
		{"doPQqPTnGxVBsagnrVV2KnFdquRA6tgdRQ",	5920000000000},
		{"dsJbNVc2zWw9ufvPyyefbAExkZZdEoQxBS",	5083333333333},
		{"ddQGMYmLjQyP5SQMxgrKTxCWNExBYmR4uL",	30500000000000}, //
		{"dmuN4nEUzYpU88HbdcE6D92e3vW3KRvzWR",	140500000000000},
		{"dyyHw8BtjPNGMF1E4kzJpvacNcLsbHh93x",	10166666666666},
		{"e22z8yqDnoxsrp5YSkdHFzERv9G3ZnVo2u",	121166666666666},
		{"djxpDZTnLfpfY6qj2LPidrEf7EHoYR3TYP",	19990000000000},

		{"e1nu3RPuipEgsoJGRxHjT9Gzj57o9DcoPY",	20000000000000},
		{"dn5qSRmqE3uaRoNEkZ28oek1pvA8Nk61j4",	20000000000000},
		{"dw5CpSdvPYsXta7gysmtMmUjE4XwdSdcx5",	10000000000000},
		{"dyCEnBvsZNfwRNFoM4TrYQAr1yKxWGFqP1",	32090000000000}//
		//{"dyY64JRvdtGTpu8KiJqfpPvLc6AShpkTiY",	34000000},
#endif
		/*=================二期IPO第一批发币==============================
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
		/*=================二期IPO第二批发币==============================
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
		*/
		/*=================二期IPO第三批发币==============================*/
/*		{"DUf8eyPR6XdNrzryd5J61HbKHvSMsMB6AG",	241805652793523   },
		{"Dppfoo66R2JnHyGaYHDVAQeEh4VNycv5Jw",	237895142961445   },
		{"DagEgfLcoBch6QHjBHZwGv3d8a7yNUZvq4",	101203390268344   },
		{"DeJ7imC7baVdsU3d9uvwGPXTSQ8xKVtXAa",	18856210281645    },
		{"DkE7fomsnLLZ2ZkvYFjTFhYuYgLQpeWAxH",	32240753705803    },
		{"Dd8Gf1jN89aUGX4dArUzxircV2b7uszfTn",	31717534410533    },
		{"DaZpxWaHaeqNGZRyeDomyNrmQ4oZyiiHu9",	156095846904437   },
		{"DdNwyLCar7cVYjQRgziuEvfrFmbU2JpMNY",	609764184032080   },
		{"DdqwdjxAL1p6ZfmkUKbwdyTVFSTm5Np8CC",	15609584690444    },
		{"DW4pTBfyGjmZTGtkcGNtzhxXRHmf7FbYMv",	15490516249954    },
		{"DsiQo7FfWx7oGchPmARQcKqeTdy1ZRNjc6",	63459923086691    },
		{"Do5jPqr8G9iG3P8CcqtAZocaV6SwDzNAUu",	153011360252777   },
		{"DZHn8N2gkDTh3wYSzKjwhbgbyTeYz7KpjN",	55248479022527    },
		{"DayfBVQ8qD6raMLVKhDCoKJzuRjBRe5ZJj",	494068063432120   },
		{"DmxSaF392Ud9ZpY52zsgc6NyvaUz8omFf4",	173341686391992   },
		{"Dp9LVZSX2CTREytdYkUCzP8iZjKRK63ckf",	9435968447862     },
		{"Di8U4vztuxxFnqwR4RzDZ1HbDV9dqy2oiA", 	23100000000000    },
		{"DYMaTEBoYtuFXGPsjMtdb7wr5xEVqETzyW",	68037287472000    },
		{"De6fbY6YhXLdwYxnHtn5UaLz5sv3AnYgAF",	89040000000000    },
		{"DYU1ACJNFRMiJ8hjmJrTPWPsEmNrmPAYL5",	44520000000000    },
		{"Dc6ChgzzEHyWWgPYM7WFVfebo33naQL67n",	43680000000000    }
*/

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



#define max_user   300 //100


static IPO_USER userarray[max_user];
CIpoTest::CIpoTest():nNum(0), nStep(0), strTxHash(""), strAppRegId("") {

}

TEST_STATE CIpoTest::Run(){
	int64_t nMoneySend(0);
	size_t t_num = sizeof(arrayData) / sizeof(arrayData[0]);
	BOOST_CHECK(t_num <= max_user);         //防止越界
	//初始化地址表
	for (size_t i = 0; i < t_num; i++) {
		memcpy((char*)userarray[i].address,(char*)arrayData[i].pAddress,sizeof(userarray[i].address));
		userarray[i].money = arrayData[i].nMoney;
		userarray[i].freeMothmoney = arrayData[i].nMoney / 12;
		userarray[i].freemoney = userarray[i].money - userarray[i].freeMothmoney * (12 - 1);
		nMoneySend += userarray[i].money;  //统计总金额

//		cout<<"newaddr"<<i<<"address="<<userarray[i].address<<endl;
//		cout<<"newaddr"<<i<<"money="<<userarray[i].money<<endl;
//		cout<<"newaddr"<<i<<"freemoney="<<userarray[i].freemoney<<endl;
//		cout<<"newaddr"<<i<<"freeMothmoney="<<userarray[i].freeMothmoney<<endl;
	}
	BOOST_CHECK(nMoneySend == totalSendMoney);

#if 0
    // 注册ipo脚本
	RegistScript();

	/// 等待ipo脚本被确认到block中
	while(true)
	{
		if(WaitComfirmed(strTxHash, strAppRegId)) {
					break;
				}
	}
#else
	strAppRegId = "50725-1";  //"2-1"
#endif

#if 0
	/// 给每个地址转一定的金额
	int64_t money = COIN;
	size_t t_num = sizeof(arrayData) / sizeof(arrayData[0]);
	BOOST_CHECK(t_num <= max_user);         //防止越界
	for(int i=0;i <t_num;i++)
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
#endif
	cout<<"SendIpoTx start"<<endl;
	SendIpoTx(0);
	cout<<"SendIpoTx end"<<endl;
	return end_state;
}

void CIpoTest::RunIpo(unsigned char type){
	int64_t nMoneySend(0);
	size_t t_num = sizeof(arrayData) / sizeof(arrayData[0]);
	BOOST_CHECK(t_num <= max_user);         //防止越界
	//初始化地址表
	for (size_t i = 0; i < t_num; i++) {
		memcpy((char*)userarray[i].address,(char*)arrayData[i].pAddress,sizeof(userarray[i].address));
		if(!strcmp((char *)userarray[i].address,"DnKUZMvwXfprFCKhnsWRsbJTNnRZg88T2F") ||
			!strcmp((char *)userarray[i].address, "DftLSeJrMjJJ3UPeehNgArhcoAuDN5422E") ||
			!strcmp((char *)userarray[i].address, "Dg2dq98hcm84po3RX354SzVyE6DLpxq3QR") ||
			!strcmp((char *)userarray[i].address, "Dpjs5pvXmZbVt3uDEfBrMNbCsWjJzjm8XA") ||
			!strcmp((char *)userarray[i].address, "DZYDEn8CZuwgJ6YS6Zm7VvKaFc6E6tGstz") ||
			!strcmp((char *)userarray[i].address, "DcyumTafQsSh4hJo4V6DaS23Dd2QnpMXKH")) {
			userarray[i].freemoney = 0;  //自由金额已领
		}
		else {
			userarray[i].freemoney = arrayData[i].nMoney;
		}
		 //16696666666666610
		userarray[i].freeMothmoney = arrayData[i].nMoney;
		if(type == 1)
		{  //冻结1次
			userarray[i].money = arrayData[i].nMoney;
		}else{ // 冻结11次 改为冻结10次
			userarray[i].money = userarray[i].freeMothmoney * 10 + userarray[i].freemoney;
		}
		nMoneySend += userarray[i].money;  //统计总金额

	}
	BOOST_CHECK(nMoneySend == totalSendMoney);

//	//// main网络不用
//	/// 给每个地址转一定的金额
//	int64_t money = COIN;
//	t_num = sizeof(arrayData) / sizeof(arrayData[0]);
//	BOOST_CHECK(t_num <= max_user);         //防止越界
//	for(int i=0;i <t_num;i++)
//	{
//		string des =strprintf("%s", userarray[i].address);
//		basetest.CreateNormalTx(des,money);
//	}
//
//	 cout<<"end mempool"<<endl;
//	while(true)
//	{
//		if(basetest.IsMemoryPoolEmpty())
//			break;
//		MilliSleep(100);
//	}


//	strAppRegId = "97792-1";  //"2-1"

   cout<<"SendIpoTx start"<<endl;
	SendIpoTx(type);
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
	Value regscript = basetest.RegisterAppTx(regAddr, strFileName, nCurHight, nFee+1*COIN);//20
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

	Value  retValue = basetest.CreateContractTx(strAppRegId, SEND_A, buffer, 0, COIN, llSendTotal);
	if(basetest.GetHashFromCreatedTx(retValue, strTxHash)){
			return true;
	}
	return false;
}
bool CIpoTest::SendIpoTx(unsigned char type)
{
	strAppRegId = "97792-1";

	// 创建转账交易并且保存转账交易的hash
	Object objRet;
	Array SucceedArray;
	Array UnSucceedArray;
	ofstream file("ipo1_ret", ios::out | ios::ate);
	if (!file.is_open())
		throw JSONRPCError(RPC_INVALID_PARAMETER, "Cannot open wallet dump file");

	map<string, string> mapTxHash;
	size_t t_num = sizeof(arrayData) / sizeof(arrayData[0]);
	BOOST_CHECK(t_num <= max_user);         //防止越界
	for(size_t i =0;i <t_num;i++)
	{
		string des = strprintf("%s", userarray[i].address);
		int64_t nMoney = userarray[i].money;   //领币的总金额
		Object obj;

		CDataStream scriptData(SER_DISK, CLIENT_VERSION);
		scriptData << userarray[i];
		string sendcontract = HexStr(scriptData);
		if(CreateIpoTx(sendcontract,userarray[i].money)){
			mapTxHash[des]= strTxHash;
			obj.push_back(Pair("addr", des));
			obj.push_back(Pair("amount", nMoney));
			obj.push_back(Pair("txhash", strTxHash));
			SucceedArray.push_back(obj);
			cout<<"after SendIpoTx strTxHash="<<strTxHash<<endl;
		} else {
			obj.push_back(Pair("addr", des));
			obj.push_back(Pair("amount", nMoney));
			UnSucceedArray.push_back(obj);
			cout<<"after SendIpoTx strTxHash err"<<endl;
		}
	}
	objRet.push_back(Pair("succeed", SucceedArray));
	objRet.push_back(Pair("unsucceed", UnSucceedArray));
	file << json_spirit::write_string(Value(objRet), true).c_str();
	file.close();

	 cout<<"SendIpoTx wait tx is confirmed"<<endl;
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

	cout<<"after SendIpoTx,check the balance of every address "<<endl;
	//校验发币后，各个地址的账户金额和冻结金额
	for (size_t i = 0; i < t_num; ++i) {

		uint64_t acctValue = basetest.GetBalance(arrayData[i].pAddress);
		cout<<"SendIpoTx addr:"<< arrayData[i].pAddress<<" balance="<<acctValue<<" freemoney="<<userarray[i].freemoney<<endl;
		BOOST_CHECK(acctValue >= (uint64_t)userarray[i].freemoney);

		// 校验每个月的冻结金额
		Value  retValue = basetest.GetAppAccountInfo(strAppRegId,arrayData[i].pAddress);
		Value  result = find_value(retValue.get_obj(), "vFreezedFund");
		Array array = result.get_array();
//		int64_t nMoneySend(0);
		size_t j = 0;
		cout<<"SendIpoTx freeMonthNum="<<array.size()<<endl;
        for(j = 0;j < array.size();j++)
        {
        	int64_t freedmoney = find_value(array[j].get_obj(), "value").get_int64();
        	cout<<"after SendIpoTx src="<<userarray[i].freeMothmoney <<" dest="<<freedmoney<<endl;
        	BOOST_CHECK(freedmoney == userarray[i].freeMothmoney);
//        	nMoneySend += freedmoney;
        }
		if(type == 1)
		{  //冻结1次

		}else{
//            BOOST_CHECK(j == (12 - 1)); //11个冻结金额
			  BOOST_CHECK(j == (11 - 1)); //10个冻结金额
		}
	}

	return true;
}

void CIpoTest::SendErrorIopTx()
{   /*利用一个地址给自己账户充值，从脚本账户 50725-1 把钱取出来*/
	strAppRegId = "50725-1";
	IPO_USER useripo;
	char *dess = "DhxrQ9hsvo3fVVSy6By8bePt8cmPtts88R";
	memcpy((char*)useripo.address,dess,sizeof(useripo.address));
	useripo.money =1;
	useripo.freemoney = totalSendMoney+1;
	useripo.freeMothmoney = 0;
	Object obj;

	CDataStream scriptData(SER_DISK, CLIENT_VERSION);
	scriptData << useripo;
	string sendcontract = HexStr(scriptData);
	if(CreateIpoTx(sendcontract,useripo.money))
	{	cout<<"after SendIpoTx strTxHash="<<strTxHash<<endl;
	} else {
		cout<<"after SendIpoTx strTxHash err"<<endl;
	}
}
BOOST_FIXTURE_TEST_SUITE(CreateIpoTxTest,CIpoTest)

BOOST_FIXTURE_TEST_CASE(Test,CIpoTest)
{
//	Run();
	RunIpo(0); //冻结11次
//	RunIpo(1); //冻结1次
	//SendErrorIopTx();
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

BOOST_FIXTURE_TEST_CASE(check_money,CIpoTest) {
	int64_t data1(0);
	int64_t total(0);
	size_t t_num = sizeof(arrayData) / sizeof(arrayData[0]);
	BOOST_CHECK(t_num <= max_user);         //防止越界
	//初始化地址表
	for (size_t i = 0; i < t_num; i++) {
		if(!strcmp((char *)arrayData[i].pAddress,"DnKUZMvwXfprFCKhnsWRsbJTNnRZg88T2F") ||
					!strcmp((char *)arrayData[i].pAddress, "DftLSeJrMjJJ3UPeehNgArhcoAuDN5422E") ||
					!strcmp((char *)arrayData[i].pAddress, "Dg2dq98hcm84po3RX354SzVyE6DLpxq3QR") ||
					!strcmp((char *)arrayData[i].pAddress, "Dpjs5pvXmZbVt3uDEfBrMNbCsWjJzjm8XA") ||
					!strcmp((char *)arrayData[i].pAddress, "DZYDEn8CZuwgJ6YS6Zm7VvKaFc6E6tGstz") ||
					!strcmp((char *)arrayData[i].pAddress, "DcyumTafQsSh4hJo4V6DaS23Dd2QnpMXKH")) {
					data1 += arrayData[i].nMoney;
				}
		total += arrayData[i].nMoney;
	}
	total = total * 11;
	total -= data1;
	cout <<"total amount:" << total <<endl;
}
BOOST_FIXTURE_TEST_CASE(check_recharge,CIpoTest) {

	int64_t nMoneySend(0);
	size_t t_num = sizeof(arrayData) / sizeof(arrayData[0]);
	BOOST_CHECK(t_num <= max_user);         //防止越界
	//初始化地址表
	for (size_t i = 0; i < 1; i++) {
		memcpy((char*)userarray[i].address,(char*)arrayData[i].pAddress,sizeof(userarray[i].address));
		userarray[i].freemoney = arrayData[i].nMoney;
		userarray[i].freeMothmoney = arrayData[i].nMoney;
		userarray[i].money = userarray[i].freeMothmoney * 10 + userarray[i].freemoney;
	}
	//"app regid"

    cout<<"SendIpoTx start"<<endl;
	SendIpoTx(0);
    cout<<"SendIpoTx end"<<endl;
}
BOOST_AUTO_TEST_SUITE_END()

