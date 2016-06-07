#include <cstdio>
#include <cstdlib>

#include "App.h"
#include "Enclave_u.h"
#include "Log.h"
#include "EthRPC.h"

#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <string>

#include "ethrpcclient.h"
#include <jsonrpccpp/client/connectors/httpclient.h>

using namespace jsonrpc;

/*
static int rpc_base(std::string hostname, unsigned port, Json::Value& query, Json::Value& response)
{
    Json::Rpc::HttpClient HttpClient(hostname, port);
    Json::FastWriter writer;
    std::string queryStr;
    std::string responseStr;
    Json::Value res;
    Json::Reader resReader;
    std::stringstream str;

    if(!networking::init())
    {
        throw std::runtime_error("Networking initialization failed");
    }

    queryStr = writer.write(query);
    LL_LOG("query: %s", queryStr.c_str());

    if(HttpClient.Send(queryStr) != 0)
    {
        throw std::runtime_error("Error while sending data!");
    }

    if(HttpClient.Recv(responseStr) == -1)
    {
        throw std::runtime_error("Error while receiving data!");
    }

    if (!resReader.parse(responseStr, res))
    {
        std::cout << "Can't parse" << responseStr << std::endl;
        throw std::runtime_error("Parse Error");
    }

    if (!res["error"].isNull())
    {
        throw std::invalid_argument(res["error"]["message"].asString());
    }

    response = res["result"];
    
    LL_LOG("response: %s", writer.write(response).c_str());

    HttpClient.Close();
    networking::cleanup();

    return EXIT_SUCCESS;
}
*/


/**
 * \brief Entry point of the program.
 * \param argc number of argument
 * \param argv array of arguments
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */

//  curl -X POST --data '{"jsonrpc":"2.0","method":"web3_clientVersion","params":[],"id":67}' 52.23.173.127:8201

#define SEND_RAW_TX
//#undef SEND_RAW_TX

/*
int send_transaction(std::string hostname, unsigned port, char* raw)
{
  Json::Value query;
  Json::Value params;
  Json::Value resp;
  
  query["jsonrpc"] = "2.0";
  query["id"] = 1;
  params = raw;
  query["method"] = "eth_sendRawTransaction";
  query["params"][0] = params;

  rpc_base(hostname, port, query, resp);

  LL_CRITICAL("Response recorded in the blockchain.");
  LL_CRITICAL("TX: %s", resp.asCString());

  return EXIT_SUCCESS;
}
*/

HttpClient httpclient("http://localhost:8485");
ethRPCClient c(httpclient);

int send_transaction(std::string hostname, unsigned port, char* raw)
{
    ( void )(hostname);
    ( void )(port);
    std::string res;
    std::string param1(raw);
    try{
        res = c.eth_sendRawTransaction(param1);

        LL_CRITICAL("Response recorded in the blockchain.");
        LL_CRITICAL("TX: %s", res.c_str());

        return 0;
    }
    catch (JsonRpcException e) {
        LL_CRITICAL("Error in send_transaction: %s", e.what());
        return -1;
    }
}

#if 0
int eth_new_filter(std::string hostname, unsigned port, std::string& id, int from, int to)
{
    /*
    > filter_opt
    {
        address: "0x08be24cd8dcf73f8fa5db42b855b4370bd5c448b",
        fromBlock: from,
        toBlock: to,
        topics: ["0x8d2b45c22f17e6631529a8fb8f4b17f4f336d01b6db32584ec554476dbbf2af0"]
    }

    > web3.sha3("RequestInfo(uint64,uint8,address,uint256,address,bytes32,bytes32[])")
    "8d2b45c22f17e6631529a8fb8f4b17f4f336d01b6db32584ec554476dbbf2af0"

    * note that uint -> uint256 before applying SHA3
    * see: https://github.com/ethereum/wiki/wiki/Ethereum-Contract-ABI
    */
    Json::Value query;
    Json::Value filter_opt;
    Json::Value result;

    std::stringstream from_s, to_s;
    from_s << "0x" << std::hex << from;
    to_s << "0x" << std::hex << to;

    filter_opt["address"] = "0x08be24cd8dcf73f8fa5db42b855b4370bd5c448b";
    filter_opt["topics"][0] = "0x8d2b45c22f17e6631529a8fb8f4b17f4f336d01b6db32584ec554476dbbf2af0";
    filter_opt["fromBlock"] = from_s.str();
    filter_opt["toBlock"] = to_s.str();

    query["jsonrpc"] = "2.0";
    query["id"] = 1;
    query["method"] = "eth_newFilter";
    query["params"][0] = filter_opt;

    try
    {
        rpc_base(hostname, port, query, result);
    }
    catch (std::exception& re)
    {
        LL_CRITICAL("%s", re.what());
        std::cout << result << std::endl;
        return -1;
    }
    id = result.asString();
    return EXIT_SUCCESS;
}
#endif

int eth_new_filter(std::string hostname, unsigned port, std::string& id, int from, int to)
{
    Json::Value filter_opt;
    try {
        filter_opt["address"] = "0x08be24cd8dcf73f8fa5db42b855b4370bd5c448b";
        filter_opt["topics"][0] = "0x8d2b45c22f17e6631529a8fb8f4b17f4f336d01b6db32584ec554476dbbf2af0";
        filter_opt["fromBlock"] = from;
        filter_opt["toBlock"] = to;

        id = c.eth_newFilter(filter_opt);
        return 0;
    }
    catch (JsonRpcException e) {
        LL_CRITICAL("%s", e.what());
        return -1;
    }
}

#if 0
int eth_getfilterlogs(std::string hostname, unsigned port, std::string filter_id, Json::Value& result)
{
    Json::Value query;
    Json::FastWriter writer;
    // int to hex string

    std::stringstream filter_id_s;
    filter_id_s << "0x" << std::hex << filter_id;

    query["jsonrpc"] = "2.0";
    query["id"] = 1;
    query["method"] = "eth_getFilterLogs";
    query["params"][0] = filter_id;

    try
    {
        rpc_base(hostname, port, query, result);
    }
    catch (std::exception& re)
    {
        LL_CRITICAL("%s", re.what());
        return -1;
    }
    return EXIT_SUCCESS;
}
#endif

int eth_getfilterlogs(std::string hostname, unsigned port, std::string filter_id, Json::Value& result) {
    try {
        result = c.eth_getFilterLogs(filter_id);
        return 0;
    }
    catch (JsonRpcException e) {
        LL_CRITICAL("%s", e.what());
        return -1;
    }
}

#if 0
unsigned long eth_blockNumber(std::string hostname, unsigned port)
{
    Json::Value query;
    Json::FastWriter writer;

    query["jsonrpc"] = "2.0";
    query["id"] = 1;
    query["method"] = "eth_blockNumber";

    LL_DEBUG("method: %s", query["method"].asString().c_str());

    Json::Value resp;
    rpc_base(hostname, port, query, resp);

    if (resp.isString())
    {
        std::string blockNumber = resp.asString();
        return std::strtoul(blockNumber.c_str(), NULL, 16);
    }
        
    return -1;
}
#endif

unsigned long eth_blockNumber(std::string hostname, unsigned port)
{
    try {
        return c.eth_blockNumber();
    }
    catch (JsonRpcException e){
        LL_CRITICAL("%s", e.what());
        return -1;
    }
}
