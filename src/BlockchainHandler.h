#pragma once
#include <Arduino.h>
#include <memory>
#ifndef UNIT_TEST
  #include <WiFi.h>  // For WiFi status checks
  #include <HTTPClient.h>
#endif

#include <string>
#include <functional>
#include <ArduinoJson.h>
#include "EncryptionHandler.h"

// Define an enumeration for status codes
enum class BlockchainStatus {
    SUCCESS,
    FAILURE,
    NO_WIFI,
    HTTP_ERROR,
    EMPTY_RESPONSE,
    PARSING_ERROR,
    NODE_NOT_FOUND,
    READY,
    NOT_DUE,
};

struct TransferParams {
    String receiver;
    String amount;
    String tokenContract;
};

// Meshtastic callbacks
using PacketIdGenerator = std::function<uint32_t(void)>;
using SecretCallback = std::function<void(uint32_t packetId)>;

class BlockchainHandler
{
  public:
    /**
     * Initializes a new instance of the BlockchainHandler class with specified public and private keys.
     *
     * @param public_key The public key to be used for blockchain operations.
     * @param private_key The private key to be used for blockchain operations.
     * @param server_url The blockchain server URL (optional)
     */
    BlockchainHandler(const std::string& public_key, 
                     const std::string& private_key,
                     bool is_wallet_enabled,
                     const String& server_url = "http://kda.crankk.org/chainweb/0.0/mainnet01/chain/19/pact/api/v1/");

    /**
     * Destructor for the BlockchainHandler class.
     */
    ~BlockchainHandler() = default;

    /**
     * Checks if the wallet configuration is valid.
     *
     * This method verifies if the wallet is enabled and both the public and private keys are of the correct length.
     *
     * @return True if the wallet configuration is valid, otherwise false.
     */
    bool isWalletConfigValid();

    /**
     * Initiates a synchronization process with the blockchain, executing required actions based on the node's current state.
     * 
     * This method is responsible for ensuring the node's data is up-to-date with the blockchain by performing synchronization
     * tasks. It may involve sending or receiving data to/from the blockchain, depending on the node's status.
     * @param node_id The ID of the node to sync
     * @param packetIdGen Callback function to generate unique packet IDs
     * @param onSecretGen Callback function called when a secret is generated
     * @return The interval in milliseconds before the next synchronization attempt should occur.
     */
    int32_t performNodeSync(const std::string& node_id,
                           PacketIdGenerator packetIdGen = nullptr,
                           SecretCallback onSecretGen = nullptr);

    /**
     * Executes a specified command on a blockchain web service.
     *
     * This method sends a command to a blockchain-related web service and retrieves the response.
     * It is used to interact with blockchain operations through web service APIs.
     *
     * @param commandType Identifies the web service for the call.
     * @param command Specifies the blockchain command for execution on the web service.
     * @param transferParams The transfer parameters to be used for the command.
     * @return A BlockchainStatus enumeration value indicating the result of the command execution.
     */
    BlockchainStatus executeBlockchainCommand(const String &commandType, const String &command, const TransferParams& transferParams = {});

    /**
     * Encrypts a payload.
     *
     * This method encrypts the provided payload using the director's public key.
     * The encryption process involves generating a symmetric key, encrypting the payload with AES,
     * and then encrypting the symmetric key with RSA.
     *
     * @param payload The data to be encrypted.
     * @return A string containing the encrypted payload.
     */
    String encryptPayload(const std::string &payload);

    /**
     * Executes a token transfer on the blockchain.
     *
     * @param receiver The receiver's address.
     * @param amount The amount of tokens to transfer.
     * @param tokenContract The contract address of the token to transfer.
     * @return A BlockchainStatus enum value indicating the result of the transfer.
     */
    BlockchainStatus executeTransfer(const String& receiver, const String& amount, const String& tokenContract);

    /**
     * Converts a BlockchainStatus enum value to its corresponding string representation.
     *
     * This method takes a BlockchainStatus enum value and returns a human-readable string
     * that represents the status. This is useful for logging, debugging, or displaying
     * the status in a user interface.
     *
     * @param status The BlockchainStatus enum value to be converted.
     * @return A string representation of the given BlockchainStatus.
     */
    std::string blockchainStatusToString(BlockchainStatus status);

    /**
     * Check if WiFi is available
     */
    bool isWifiAvailable() const { return WiFi.status() == WL_CONNECTED; }

  private:
    /**
     * Creates a JSON document representing a blockchain command.
     *
     * This method constructs a JSON document that encapsulates the details of a blockchain command,
     * including the command itself and associated metadata required for its execution.
     * Uses ArduinoJson's JsonDocument for efficient memory management and JSON handling.
     *
     * @param command The blockchain command to be executed.
     * @param transferParams The transfer parameters to be used for the command.
     * @return A JsonDocument representing the command to be sent to the blockchain.
     */
    JsonDocument createCommandObject(const String &command, const TransferParams& transferParams = {});

    /**
     * Prepares a JSON document for POST request based on the command object and command type.
     *
     * This method prepares a JSON document that is ready to be sent as a POST request to the blockchain.
     * It includes the command object and additional information based on the command type.
     * Uses ArduinoJson's JsonDocument for efficient memory handling and serialization.
     *
     * @param cmdObject The command object created by createCommandObject.
     * @param commandType The type of the command, affecting how the post object is prepared.
     * @return A JsonDocument ready for being sent as a POST request.
     */
    JsonDocument preparePostObject(const JsonDocument &cmdObject, const String &commandType);

    /**
     * Parses the blockchain response received as a string into a more usable form.
     *
     * This method takes a blockchain response in the form of a string, parses it using ArduinoJson,
     * and extracts relevant information, making it easier to handle the response programmatically.
     *
     * The method first attempts to parse the response string into a JSON document. If the parsing
     * fails, it returns a PARSING_ERROR status. It then examines the "result" field of the JSON
     * document to determine the status of the blockchain command. Depending on the command type,
     * it extracts specific information such as the director's public key and the send status.
     *
     * @param response The blockchain response as a raw string.
     * @param command The blockchain command that was executed, used to determine the context of the response.
     * @return A BlockchainStatus enum value representing the status of the parsed response.
     */
    BlockchainStatus parseBlockchainResponse(const String &response, const String &command);

    std::string public_key_;
    std::string private_key_;
    bool is_wallet_enabled_;
    String kda_server_;
    std::string director_pubkeyd_;
    std::unique_ptr<EncryptionHandler> encryptionHandler_;
};
