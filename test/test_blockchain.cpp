#include <unity.h>
#include "BlockchainHandler.h"


void test_invalid_wallet_config(void) {
    BlockchainHandler handler("", "", false, "http://test.url");
    TEST_ASSERT_FALSE(handler.isWalletConfigValid());
}

void test_valid_wallet_config(void) {
    std::string valid_pub_key(64, 'a');
    std::string valid_priv_key(64, 'b');
    BlockchainHandler handler(valid_pub_key, valid_priv_key, true, "http://test.url");
    TEST_ASSERT_TRUE(handler.isWalletConfigValid());
}

void test_wifi_connection(void) {
    // Test NO_WIFI status
    WiFi.setStatus(WL_DISCONNECTED);
    std::string valid_pub_key(64, 'a');
    std::string valid_priv_key(64, 'b');
    BlockchainHandler handler(valid_pub_key, valid_priv_key, true, "http://test.url");
    BlockchainStatus status = handler.executeBlockchainCommand("test", "This is a test");
    TEST_ASSERT_EQUAL(BlockchainStatus::NO_WIFI, status);

    // Test connected status
    WiFi.setStatus(WL_CONNECTED);
    status = handler.executeBlockchainCommand("test", "This is a test");
    TEST_ASSERT_EQUAL(BlockchainStatus::EMPTY_RESPONSE, status);
}

void test_transfer_create(void) {
    // Setup valid wallet
    std::string valid_pub_key(64, 'a');
    std::string valid_priv_key(64, 'b');
    std::string receiver_address(64, 'r');
    BlockchainHandler handler(valid_pub_key, valid_priv_key, true, "http://test.url");

    // Test with valid parameters
    WiFi.setStatus(WL_CONNECTED);
    BlockchainStatus status = handler.executeTransfer(
        receiver_address,
        "414.0",  // amount
        "free.crankk01"  // token contract
    );
    TEST_ASSERT_EQUAL(BlockchainStatus::EMPTY_RESPONSE, status);

    // Test with invalid amount
    status = handler.executeTransfer(
        receiver_address,
        "-10.0",
        "free.crankk01"
    );
    TEST_ASSERT_EQUAL(BlockchainStatus::INVALID_AMOUNT, status);

    // Test with invalid wallet config
    BlockchainHandler invalid_handler("", "", false, "http://test.url");
    status = invalid_handler.executeTransfer(
        receiver_address,
        "414.0",
        "free.crankk01"
    );
    TEST_ASSERT_EQUAL(BlockchainStatus::FAILURE, status);

    // Test with no WiFi
    WiFi.setStatus(WL_DISCONNECTED);
    status = handler.executeTransfer(
        receiver_address,
        "414.0",
        "free.crankk01"
    );
    TEST_ASSERT_EQUAL(BlockchainStatus::NO_WIFI, status);
}
