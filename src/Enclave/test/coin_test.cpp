//
// Copyright (c) 2016-2018 by Cornell University.  All Rights Reserved.
//
// Permission to use the "TownCrier" software ("TownCrier"), officially
// docketed at the Center for Technology Licensing at Cornell University
// as D-7364, developed through research conducted at Cornell University,
// and its associated copyrights solely for educational, research and
// non-profit purposes without fee is hereby granted, provided that the
// user agrees as follows:
//
// The permission granted herein is solely for the purpose of compiling
// the TownCrier source code. No other rights to use TownCrier and its
// associated copyrights for any other purpose are granted herein,
// whether commercial or non-commercial.
//
// Those desiring to incorporate TownCrier software into commercial
// products or use TownCrier and its associated copyrights for commercial
// purposes must contact the Center for Technology Licensing at Cornell
// University at 395 Pine Tree Road, Suite 310, Ithaca, NY 14850; email:
// ctl-connect@cornell.edu; Tel: 607-254-4698; FAX: 607-254-5454 for a
// commercial license.
//
// IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR
// DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
// INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF TOWNCRIER AND ITS
// ASSOCIATED COPYRIGHTS, EVEN IF CORNELL UNIVERSITY MAY HAVE BEEN
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// THE WORK PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND CORNELL
// UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
// ENHANCEMENTS, OR MODIFICATIONS.  CORNELL UNIVERSITY MAKES NO
// REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER IMPLIED
// OR EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR THAT THE USE
// OF TOWNCRIER AND ITS ASSOCIATED COPYRIGHTS WILL NOT INFRINGE ANY
// PATENT, TRADEMARK OR OTHER RIGHTS.
//
// TownCrier was developed with funding in part by the National Science
// Foundation (NSF grants CNS-1314857, CNS-1330599, CNS-1453634,
// CNS-1518765, CNS-1514261), a Packard Fellowship, a Sloan Fellowship,
// Google Faculty Research Awards, and a VMWare Research Award.
//

#include <stdio.h>
#include <stdlib.h>
#include <debug.h>

#include "tls_client.h"
#include "scrapers/current_coinmarket.h"
#include "../log.h"


int coin_self_test(){
	CoinMarket coinMarketTest;
	
	//Null Checker
	double r = 0.0;
	if (coinMarketTest.coinmarketcap_current(NULL, &r) != INVALID_PARAMS || coinMarketTest.coinmarketcap_current("GOOG", NULL) != INVALID_PARAMS ||\
		coinMarketTest.coinmarketcap_current(NULL, NULL) != INVALID_PARAMS){
		return -1;
	}

	//Test out regular performance
	if(coinMarketTest.coinmarketcap_current("bitcoin", &r) != NO_ERROR){
		return -1;
	}
	LL_INFO("[CoinMarket] bitcoin returns: %f", r);
	
	if (r == 0 || coinMarketTest.coinmarketcap_current("litecoin", &r) != NO_ERROR){
		return -1;
	}
	LL_INFO("[CoinMarket] litecoin returns: %f", r);

	if (r == 0 || coinMarketTest.coinmarketcap_current("ethereum", &r) != NO_ERROR){
		return -1;
	}
	LL_INFO("[CoinMarket] ethereum returns: %f", r);

	if (r == 0 || coinMarketTest.coinmarketcap_current("ripple", &r) != NO_ERROR){
		return -1;
	}
	LL_INFO("[CoinMarket] ripple returns: %f", r);


	//Null check
	int ret = coinMarketTest.coinmarketcap_current("vdbsd", &r);
	if (r != 0.0){
		return -1;
	}

	return 0;
}