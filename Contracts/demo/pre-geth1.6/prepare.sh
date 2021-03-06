cat << EOF
if (eth.accounts.length < 3)
{
    console.log(eth.accounts.length + ' accounts found. Need 3')
    exit;
}

var minerAddr = eth.accounts[0]
var sellerAddr = eth.accounts[1]
var buyerAddr = eth.accounts[2]
var sgxAddr = "0x89b44e4d3c81ede05d0f5de8d1a68f754d73d997"

var gasCnt = 3e+6
var TC_FEE = 3e+15

var encryptedApiKey = [
    '0xf68d2a32cf17b1312c6db3f236a38c94', 
    '0x4c9f92f6ec1e2a20a1413d0ac1b867a3']

var buyerSteamId = String(32884794);
EOF

# Make sure you modify the TC contract to have the right SGX address.
SRC=$(sed 's/\(\/\/.*$\|import "[^"]\+";\)//' ../TownCrier.sol | paste -sd '' | sed 's/\s\+/ /g')

cat <<EOF
var TCsource = '$SRC'
EOF

SRC=$(sed 's/\(\/\/.*$\|import "[^"]\+";\)//' ../Ref.sol ../Application.sol ../FlightInsurance.sol| paste -sd '' | sed 's/\s\+/ /g')

cat <<EOF
var APPsource = '$SRC'
EOF

cat <<EOF
var TCcontract = eth.compile.solidity(TCsource)
var APPcontracts = eth.compile.solidity(APPsource)
var TownCrier = eth.contract(TCcontract["<stdin>:TownCrier"].info.abiDefinition)
//var SteamTrade = eth.contract(APPcontracts["<stdin>:SteamTrade"].info.abiDefinition)
var FlightIns = eth.contract(APPcontracts["<stdin>:FlightInsurance"].info.abiDefinition)
var App = eth.contract(APPcontracts["<stdin>:Application"].info.abiDefinition)

function checkWork(){
    if (eth.getBlock("pending").transactions.length > 0) {
        if (eth.mining) return;
        console.log("== Pending transactions! Mining...");
        miner.start(1);
    } else {
        if (!eth.mining) return;
        miner.stop();
        console.log("== No transactions! Mining stopped.");
    }
}

function mineBlocks(num) {
    miner.start(1); miner.start(1); admin.sleepBlocks(num); miner.stop();
}


function setup_log(tc, tradeContract, id) {
if (id == 0) {
    tc.RequestInfo(function(e,r) { 
		if (!e) { console.log('TC RequestInfo: ' + JSON.stringify(r.args)); } 
		else {console.log(e)}
	});

	tc.DeliverInfo(function(e,r) { 
		if (!e) { console.log('TC ResponseInfo: ' + JSON.stringify(r.args)); } 
		else {console.log(e)}
	});

	tc.Cancel(function(e,r) { 
		if (!e) { console.log('TC Cancel: ' + JSON.stringify(r.args)); } 
		else {console.log(e)}
	});
}

    if (id == 1) {
        tradeContract.Insure(function(e,r) {
            if (!e) { console.log('App Insure: ' + JSON.stringify(r.args)); }
            else { console.log(e)}
        });
    }

    tradeContract.Request(function(e,r) { 
		if (!e) { console.log('App Request: ' + JSON.stringify(r.args)); } 
		else {console.log(e)}
	});
	
    tradeContract.Response(function(e,r) { 
		if (!e) { console.log('App Response: ' + JSON.stringify(r.args)); } 
		else {console.log(e)}
	});
	
    tradeContract.Cancel(function(e,r) { 
		if (!e) { console.log('App Cancel: ' + JSON.stringify(r.args)); } 
		else {console.log(e)}
	});
}

// TODO: watch RequestLog and print it out
// TODO: Not an emergency

function setup_tc() {
    unlockAccounts();
    var tc = TownCrier.new({
        from: minerAddr, 
        data: TCcontract["<stdin>:TownCrier"].code, 
        gas: gasCnt}, function(e, c) {
            if (!e){
                if (c.address) {
                    console.log('Town Crier created at: ' + c.address)
                }
            } 
            else {console.log('Failed to create Town Crier contract: ' + e)}
        });
    mineBlocks(1);
    return tc;
}

function createSteamTrade(apiKey, item, price) {
    unlockAccounts();
  var tradeContract = SteamTrade.new(
          tc.address, apiKey[0], apiKey[1], item, price, {
              from: sellerAddr, 
              data: APPcontracts["<stdin>:SteamTrade"].code, 
              gas: gasCnt}, 
              function(e, c) { 
                  if (!e) {
                      if (c.address) {
                        console.log('SteamTrade created at: ' + c.address)
                      }
                  } 
                  else {console.log('Failed to create SteamTrade contract: ' + e)}
              });
    mineBlocks(1);
    return tradeContract;
}

function createFlightIns(tc) {
    unlockAccounts();
    var tradeContract = FlightIns.new(
            tc, {
                value: 10e+18,
                from: sellerAddr,
                data: APPcontracts["<stdin>:FlightInsurance"].code,
                gas: gasCnt},
                function(e, c) {
                    if (!e) {
                        if (c.address) {
                            console.log('FlightIns created at: ' + c.address)
                        }
                    }
                    else {console.log('Failed to create FligthIns contract: ' + e)}
                });
    mineBlocks(1);
    return tradeContract;
}

function createApp(tc) {
    unlockAccounts();
    var tradeContract = App.new(
        tc, {
            from: sellerAddr,
            data: APPcontracts["<stdin>:Application"].code,
            gas:gasCnt},
            function(e, c) {
                if (!e) {
                    if (c.address) {
                        console.log('Application created at: ' + c.address)
                    }
                } else {
                    console.log('Failed to create Application contract: ' + e)}
                });
    mineBlocks(1);
    return tradeContract;
}

function SteamPurchase(contract, steamId, delay) {
  // var timeoutSecs = Math.floor((new Date((new Date()).getTime() + (delay * 1000))).getTime() / 1000);
  // to simplify, delay is the time for SGX to wait before fetching
  // delay = 60, typically
    unlockAccounts();
  contract.purchase.sendTransaction( steamId, delay, {
      from: buyerAddr, 
      value: 1e+18 + TC_FEE, 
      gas: gasCnt
  });
  mineBlocks(1);
  return "Purchased!"
}

function FlightInsure(contract, flightId, time, fee) {
    unlockAccounts();
contract.insure.sendTransaction([flightId, time], fee, {
        from: buyerAddr,
        value: fee * 1e+18 + TC_FEE,
        gas: gasCnt
    });
    mineBlocks(1);
    return "Insured!"
}

function FlightRequest(contract, id) {
    unlockAccounts();
    contract.request.sendTransaction(id, {
        from: buyerAddr,
        gas: gasCnt
    });
    mineBlocks(1);
    return "Request sent!"
}

function FlightCancel(contract, id) {
    unlockAccounts();
    contract.cancel.sendTransaction(id, {
        from: buyerAddr,
        gas: gasCnt
    });
    mineBlocks(1);
    return "Request Cancelled."
}

function Request(contract, type, requestData) {
    unlockAccounts();
    contract.request.sendTransaction(type, requestData, {
        from: buyerAddr,
        value: 3e15,
        gas: gasCnt
    });
    mineBlocks(1);
    return "Request sent!";
}

function Cancel(contract, id) {
    unlockAccounts();
    contract.cancel.sendTransaction(id, {
        from: buyerAddr,
        gas: gasCnt
    });
    mineBlocks(1);
    return "Request Cancelled.";
}

function TestSteam(contract, steamId, delay) {
    unlockAccounts();
    for (var i = 0; i < 1000; ++i) {
        SteamPurchase(contract, steamId, delay);
    }
}


function check_balance(){
    var before = Number(eth.getBalance(sellerAddr));
    var before_b = Number(eth.getBalance(buyerAddr));

    mineBlocks(1);

    var after = Number(eth.getBalance(sellerAddr));
    var after_b = Number(eth.getBalance(buyerAddr));

    console.log('seller balance before: ' + before*1e-18 + ' ether');
    console.log('seller balance after: ' + after*1e-18 + ' ether');
    console.log('balance delta: ' + (after - before)*1e-18 + ' ether');

    console.log('buyer balance before: ' + before_b*1e-18 + ' ether');
    console.log('buyer balance after: ' + after_b*1e-18 + ' ether');
    console.log('balance delta: ' + (after_b - before_b)*1e-18 + ' ether');

    return "Success!"
}

function watch_events(contract) {
    var his = contract.allEvents({fromBlock: 0, toBlock: 'latest'});
    var events;
    his.get(function(error, result) {
        if (!error) {
            console.log(result.length);
            for (var i = 0; i < result.length; ++i) {
                console.log(i + " : " + result[i].event);
            }
            events = result;
        } else {
            console.log("error");
            events = "error";
        }
    });
    return events;
}


function unlockAccounts() {
    for (var i = 0; i < eth.accounts.length; ++i) {
        personal.unlockAccount(eth.accounts[i], '123123');
    }
}

function pad(n, width) {
    m = n.toString(16);
    return '0x' + new Array(width - m.length + 1).join('0') + m;
}

/* =========== The following should be run line-by-line as a demo =========== */
// loadScript("demorc.js");
// tc = setup_tc();
// app = createApp(tc.address);
// setup_log(tc, app, 0);
// Request(app, 1, ['FJM273', pad(1492100100, 64)]);
// Request(app, 2, ['f68d2a32cf17b1312c6db3f236a38c94', '4c9f92f6ec1e2a20a1413d0ac1b867a3', '32884794', pad(1456380265, 64), pad(1, 64), 'Portal']);
// Request(app, 3, ['GOOG', pad(1262390400,64)]);
// Request(app, 4, ['1ZE331480394808282']);
// Request(app, 5, ['bitcoin']);
// FlightInsure(fi, 'FJM273', pad(1492100100, 64), 1);

// var tradeContract = createSteamTrade(encryptedApiKey, 'Portal', 1e+18);
// TestSteam(tradeContract, buyerSteamId, 60);
// var insContract = createFlightIns();
// FlightInsure(insContract, "fightID", "epochtime", 5);
EOF
