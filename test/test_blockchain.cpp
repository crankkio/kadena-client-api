#include <unity.h>
#include "BlockchainHandler.h"
#include <ArduinoJson.h>


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
    String postRaw;
    BlockchainStatus status = handler.executeBlockchainCommand("send", "This is a test", postRaw);
    TEST_ASSERT_EQUAL(BlockchainStatus::NO_WIFI, status);

    // Test connected status
    WiFi.setStatus(WL_CONNECTED);
    String postRaw2;
    status = handler.executeBlockchainCommand("local", "This is a test", postRaw2);
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
    String transferString;
    BlockchainStatus status = handler.executeTransfer(
        receiver_address,
        "414.0",  // amount
        "free.crankk01",
        transferString
    );
    TEST_ASSERT_EQUAL(BlockchainStatus::EMPTY_RESPONSE, status);

    // Test with invalid amount
    status = handler.executeTransfer(
        receiver_address,
        "-10.0",
        "free.crankk01",
        transferString
    );
    TEST_ASSERT_EQUAL(BlockchainStatus::INVALID_AMOUNT, status);

    // Test with invalid wallet config
    BlockchainHandler invalid_handler("", "", false, "http://test.url");
    status = invalid_handler.executeTransfer(
        receiver_address,
        "414.0",
        "free.crankk01",
        transferString
    );
    TEST_ASSERT_EQUAL(BlockchainStatus::FAILURE, status);

    // Test with no WiFi
    WiFi.setStatus(WL_DISCONNECTED);
    String transferStringNoWifi;
    status = handler.executeTransfer(
        receiver_address,
        "42.0",
        "free.crankk01",
        transferStringNoWifi
    );
    TEST_ASSERT_EQUAL(BlockchainStatus::NO_WIFI, status);

    // Parse the transfer string to verify its contents
    JsonDocument doc;
    deserializeJson(doc, transferStringNoWifi);

    // Get the inner command object (it's a string that needs to be parsed again)
    JsonDocument cmdDoc;
    deserializeJson(cmdDoc, doc["cmds"][0]["cmd"]);

    // Now we can check specific values
    float amount = cmdDoc["signers"][0]["clist"][1]["args"][2];
    String contract = cmdDoc["signers"][0]["clist"][1]["name"];
    String receiver = cmdDoc["signers"][0]["clist"][1]["args"][1];
    TEST_ASSERT_EQUAL(42.0, amount);
    TEST_ASSERT_EQUAL_STRING("free.crankk01.TRANSFER", contract.c_str());
    TEST_ASSERT_EQUAL_STRING("k:rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr", receiver.c_str());
}
