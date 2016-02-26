#include "stdint.h"
#include "EventHandler.h"
#include <string>
#include <Log.h>
#include "dispatcher.h"
#include "time.h"
#include "Transaction.h"
#include "ABI.h"
#include "Enclave_t.h"
#include "keccak.h"
#include <Debug.h>
#include "Constants.h"

static int stock_ticker_handler(int nonce, uint64_t request_id, uint8_t request_type, 
                   const uint8_t* req, int req_len, uint8_t* raw_tx, int* raw_tx_len)
{
    int ret;
#ifdef E2E_BENCHMARK
    long long time1, time2;
    rdtsc(&time1);
    LL_CRITICAL("swtich in done: %llu", time1);
#endif
    if (req_len != 64)
    {
        LL_CRITICAL("req_len is not 64");
        return -1;
    }
    
    char* code = (char*)(req);
    uint32_t date;
    time_t epoch;
    memcpy(&date, req + 64 - sizeof uint32_t, sizeof uint32_t);
    date = swap_uint32(date);

    epoch = date;
    LL_NOTICE("Looking for %s at %lld", code, epoch);
#ifdef E2E_BENCHMARK
    rdtsc(&time1);
#endif
    int price = (int) get_closing_price(12, 3, 2014, "BABA");
#ifdef E2E_BENCHMARK
    rdtsc(&time2);
    LL_CRITICAL("get_closing_price: %llu", time2 - time1);
#endif
    LL_NOTICE("Closing pricing is %d", price);

    bytes rr;
    enc_int(rr, price, sizeof price);;

    ret = get_raw_signed_tx(nonce, 32, 
        request_id, request_type, 
        req, req_len, 
        &rr[0], 32, 
        raw_tx, raw_tx_len);
#ifdef E2E_BENCHMARK
    rdtsc(&time1);
    LL_CRITICAL("swtich out begins:  %llu", time1);
#endif
    return ret;
}

static int flight_insurance_handler()
{

    int ret, delay;
#ifdef E2E_BENCHMARK
    long long time1, time2;
    rdtsc(&time1);
    LL_CRITICAL("swtich in done:  %llu", time1);
#endif
    ret = get_flight_delay("20160215", "0655", "UAL1183", &delay);
#ifdef E2E_BENCHMARK
    rdtsc(&time2);
    LL_CRITICAL("get_flight_delay: %llu", time2-time1);
#endif
    LL_NOTICE("delay is %d", delay);

    bytes rr;
    enc_int(rr, delay, sizeof delay);;

    uint8_t req[64];
    uint8_t raw_tx[1024];
    int raw_tx_len = sizeof raw_tx;
    uint8_t nonce = 1;

    ret = get_raw_signed_tx(nonce, 32, 
        1, 1, 
        req, sizeof req, 
        &rr[0], 32, 
        raw_tx, &raw_tx_len);

#ifdef E2E_BENCHMARK
    rdtsc(&time1);
    LL_CRITICAL("swtich out begins:  %llu", time1);
#endif
    return ret;
}

static int steam_exchange(uint8_t* req, int len, int* resp_data)
{
    /*
    format[0] = encAPI[0];
    format[1] = encAPI[1];
    format[2] = ID_B;
    format[3] = T_B;
    format[4] = bytes32(LIST_I.length);
    format[5] = LIST_I[0];
    */
    int ret, rc;
#ifdef E2E_BENCHMARK
    long long time1, time2;
    rdtsc(&time1);
    LL_CRITICAL("swtich in done:  %llu", time1);
#endif
    /* handling input */
    hexdump("encAPI:", req, 32);
    LL_NOTICE("buyer id: %s", "32884794");
    
    uint32_t sleep_time;
    memcpy(&sleep_time, req + 3 * 32, sizeof uint32_t);

    LL_NOTICE("item: %s", (char*)req + 5*32);

    // TODO: reading sleep_time from bytes32 is still not right
    // e.g. 10 seconds results in A0 00 00 .. 00.
    // how is int -> bytes32 transformation works?
    if (sleep_time > 60)
    {
        sleep_time = 10;
    }
    if (sleep_time > 3600)
        sleep_time = 59;
    LL_NOTICE("waiting time: %d", sleep_time);

    char * listB[1] = {"Portal"};
    rc = get_steam_transaction(listB, 1, "32884794", sleep_time, "7978F8EDEF9695B57E72EC468E5781AD", &ret);
    if (rc == 0 && ret == 1) {
        LL_NOTICE("Found a trade, %d, %d", rc, ret);
        *resp_data = ret;
        return 0;
    }

#ifdef E2E_BENCHMARK
    rdtsc(&time2);
    LL_CRITICAL("get_flight_delay: %llu", time2-time1);
    rdtsc(&time1);
    LL_CRITICAL("swtich out begins:  %llu", time1);
#endif

    return 0;
}

//request(uint8 type, address cb, bytes4 cb_fid, bytes32[] req)
int handle_request(int nonce, uint64_t request_id, uint8_t request_type, 
                   uint8_t* req, int req_len, uint8_t* raw_tx, int* raw_tx_len)
{
    int ret;
    bytes resp_data;
    int resp_data_len = 0;
    // note that length is in byte



    switch (request_type)
    {
    case TYPE_FINANCE_INFO:
        return stock_ticker_handler(nonce, request_id, request_type, 
            req, req_len, 
            raw_tx, raw_tx_len);
        break;
    case TYPE_FLIGHT_INS:
        return flight_insurance_handler();
    case TYPE_STEAM_EX:
        {

            int found = 0;
            if (req_len != 6 * 32)
            {
                LL_CRITICAL("req_len %d is not 6*32", req_len);
                return -1;
            }
            ret = steam_exchange(req, req_len, &found);
            if (ret != 0)
            {
                LL_CRITICAL("%s returns %d", "steam_exchange", ret);
                return -1;
            }
//            found = 1;
            enc_int(resp_data, found, sizeof found);
            resp_data_len = 32;
            break;
        }
    case TYPE_CURRENT_VOTE:
        {
        double r1 = 0, r2 = 0, r3 = 0;
        long long time1, time2;

        rdtsc(&time1);
        yahoo_current("GOOG", &r1);
        rdtsc(&time2);
        LL_CRITICAL("Yahoo: %llu", time2-time1);

        google_current("GOOG", &r3);
        rdtsc(&time1);
        LL_CRITICAL("Bloomberg: %llu", time1-time2);

        google_current("GOOG", &r2);
        rdtsc(&time2);
        LL_CRITICAL("GOOGLE: %llu", time2-time1);

        break;
        }
    default :
        LL_CRITICAL("Unknown request type: %d", request_type);
        return -1;
        break;
    }

    ret = get_raw_signed_tx(nonce, 32, request_id, request_type, req, req_len, &resp_data[0], resp_data_len, raw_tx, raw_tx_len);
    return ret;
}

