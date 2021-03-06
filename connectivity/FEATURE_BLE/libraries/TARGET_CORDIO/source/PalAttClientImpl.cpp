/* mbed Microcontroller Library
 * Copyright (c) 2006-2020 ARM Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ble/internal/PalAttClient.h"
#include "ble/GattServer.h"
#include "ble/internal/PalSimpleAttServerMessage.h"
#include "ble/internal/PalGap.h"
#include "ble/internal/PalGattClient.h"
#include "ble/internal/BLEInstanceBase.h"

#include "att_api.h"
#include "att_defs.h"

namespace ble {

PalAttClient::PalAttClient() : _local_sign_counter(0) {}

PalAttClient::~PalAttClient() {}

/**
* @see ble::PalAttClient::exchange_mtu_request
*/
ble_error_t PalAttClient::exchange_mtu_request(connection_handle_t connection)
{
    AttcMtuReq(connection, pAttCfg->mtu);
    return BLE_ERROR_NONE;
}

/**
* @see ble::PalGattClient::get_mtu_size
*/
ble_error_t PalAttClient::get_mtu_size(
    connection_handle_t connection_handle,
    uint16_t &mtu_size
)
{
    mtu_size = AttGetMtu(connection_handle);
    return BLE_ERROR_NONE;
}

/**
* @see ble::PalAttClient::find_information_request
*/
ble_error_t PalAttClient::find_information_request(
    connection_handle_t connection_handle,
    attribute_handle_range_t discovery_range
)
{
    AttcFindInfoReq(
        connection_handle,
        discovery_range.begin,
        discovery_range.end,
        false
    );
    return BLE_ERROR_NONE;
}

/**
* @see ble::PalAttClient::find_by_type_value_request
*/
ble_error_t PalAttClient::find_by_type_value_request(
    connection_handle_t connection_handle,
    attribute_handle_range_t discovery_range,
    uint16_t type,
    const Span<const uint8_t> &value
)
{
    AttcFindByTypeValueReq(
        connection_handle,
        discovery_range.begin,
        discovery_range.end,
        type,
        value.size(),
        const_cast<uint8_t *>(value.data()),
        false
    );
    return BLE_ERROR_NONE;
}

/**
* @see ble::PalAttClient::read_by_type_request
*/
ble_error_t PalAttClient::read_by_type_request(
    connection_handle_t connection_handle,
    attribute_handle_range_t read_range,
    const UUID &type
)
{
    AttcReadByTypeReq(
        connection_handle,
        read_range.begin,
        read_range.end,
        type.getLen(),
        const_cast<uint8_t *>(type.getBaseUUID()),
        false
    );
    return BLE_ERROR_NONE;
}

/**
* @see ble::PalAttClient::read_request
*/
ble_error_t PalAttClient::read_request(
    connection_handle_t connection_handle,
    attribute_handle_t attribute_handle
)
{
    AttcReadReq(connection_handle, attribute_handle);
    return BLE_ERROR_NONE;
}

/**
* @see ble::PalAttClient::read_blob_request
*/
ble_error_t PalAttClient::read_blob_request(
    connection_handle_t connection_handle,
    attribute_handle_t attribute_handle,
    uint16_t offset
)
{
    AttcReadLongReq(
        connection_handle,
        attribute_handle,
        offset,
        false
    );
    return BLE_ERROR_NONE;
}

/**
* @see ble::PalAttClient::read_multiple_request
*/
ble_error_t PalAttClient::read_multiple_request(
    connection_handle_t connection_handle,
    const Span<const attribute_handle_t> &attribute_handles
)
{
    AttcReadMultipleReq(
        connection_handle,
        attribute_handles.size(),
        const_cast<uint16_t *>(attribute_handles.data())
    );
    return BLE_ERROR_NONE;
}

/**
* @see ble::PalAttClient::read_by_group_type_request
*/
ble_error_t PalAttClient::read_by_group_type_request(
    connection_handle_t connection_handle,
    attribute_handle_range_t read_range,
    const UUID &group_type
)
{
    AttcReadByGroupTypeReq(
        connection_handle,
        read_range.begin,
        read_range.end,
        group_type.getLen(),
        const_cast<uint8_t *>(group_type.getBaseUUID()),
        false
    );
    return BLE_ERROR_NONE;
}

/**
* @see ble::PalAttClient::write_request
*/
ble_error_t PalAttClient::write_request(
    connection_handle_t connection_handle,
    attribute_handle_t attribute_handle,
    const Span<const uint8_t> &value
)
{
    AttcWriteReq(
        connection_handle,
        attribute_handle,
        value.size(),
        const_cast<uint8_t *>(value.data())
    );
    return BLE_ERROR_NONE;
}

/**
* @see ble::PalAttClient::write_command
*/
ble_error_t PalAttClient::write_command(
    connection_handle_t connection_handle,
    attribute_handle_t attribute_handle,
    const Span<const uint8_t> &value
)
{
    AttcWriteCmd(
        connection_handle,
        attribute_handle,
        value.size(),
        const_cast<uint8_t *>(value.data())
    );
    return BLE_ERROR_NONE;
}

/**
* @see ble::PalAttClient::signed_write_command
*/
ble_error_t PalAttClient::signed_write_command(
    connection_handle_t connection_handle,
    attribute_handle_t attribute_handle,
    const Span<const uint8_t> &value
)
{
    AttcSignedWriteCmd(
        connection_handle,
        attribute_handle,
        _local_sign_counter,
        value.size(),
        const_cast<uint8_t *>(value.data())
    );
    _local_sign_counter++;
    return BLE_ERROR_NONE;
}

/**
* Initialises the counter used to sign messages. Counter will be incremented every
* time a message is signed.
*
* @param sign_counter initialise the signing counter to this value
*/
void PalAttClient::set_sign_counter(
    sign_count_t sign_counter
)
{
    _local_sign_counter = sign_counter;
}

/**
* @see ble::PalAttClient::prepare_write_request
*/
ble_error_t PalAttClient::prepare_write_request(
    connection_handle_t connection_handle,
    attribute_handle_t attribute_handle,
    uint16_t offset,
    const Span<const uint8_t> &value
)
{
    AttcPrepareWriteReq(
        connection_handle,
        attribute_handle,
        offset,
        value.size(),
        const_cast<uint8_t *>(value.data()),
        false,
        false
    );
    return BLE_ERROR_NONE;
}

/**
* @see ble::PalAttClient::execute_write_request
*/
ble_error_t PalAttClient::execute_write_request(
    connection_handle_t connection_handle,
    bool execute
)
{
    AttcExecuteWriteReq(
        connection_handle,
        execute
    );
    return BLE_ERROR_NONE;
}

/**
* @see ble::PalAttClient::initialize
*/
ble_error_t PalAttClient::initialize()
{
    return BLE_ERROR_NONE;
}

/**
* @see ble::PalAttClient::terminate
*/
ble_error_t PalAttClient::terminate()
{
    return BLE_ERROR_NONE;
}

// singleton of the ARM Cordio client
PalAttClient &PalAttClient::get_client()
{
    static PalAttClient _client;
    return _client;
}

void PalAttClient::when_server_message_received(
    mbed::Callback<void(connection_handle_t, const AttServerMessage &)> cb
)
{
    _server_message_cb = cb;
}

void PalAttClient::when_transaction_timeout(
    mbed::Callback<void(connection_handle_t)> cb
)
{
    _transaction_timeout_cb = cb;
}

// convert an array of byte to an uint16_t
uint16_t PalAttClient::to_uint16_t(const uint8_t *array)
{
    uint16_t result;
    memcpy(&result, array, sizeof(result));
    return result;
}

template<typename T>
bool PalAttClient::event_handler(const attEvt_t *event)
{
    if (T::can_convert(event)) {
        generated_handler(event, T::convert);
        return true;
    }
    return false;
}

bool PalAttClient::timeout_event_handler(const attEvt_t *event)
{
    if (event->hdr.status != ATT_ERR_TIMEOUT) {
        return false;
    }

    get_client().on_transaction_timeout(event->hdr.param);
    return true;
}


template<typename ResultType>
void PalAttClient::generated_handler(
    const attEvt_t *event, ResultType (*convert)(const attEvt_t *)
)
{
    get_client().on_server_event(
        event->hdr.param,
        convert(event)
    );
}

void PalAttClient::on_server_event(
    connection_handle_t connection_handle,
    const AttServerMessage &server_message
)
{
    if (_server_message_cb) {
        _server_message_cb(connection_handle, server_message);
    }
}

/**
* Upon transaction timeout an implementation shall call this function.
*
* @param connection_handle The handle of the connection of the transaction
* which has times out.
*
* @note see BLUETOOTH SPECIFICATION Version 5.0 | Vol 3, Part F Section 3.3.3
*/
void PalAttClient::on_transaction_timeout(
    connection_handle_t connection_handle
)
{
    if (_transaction_timeout_cb) {
        _transaction_timeout_cb(connection_handle);
    }
}

void PalAttClient::att_client_handler(const attEvt_t *event)
{
#if BLE_FEATURE_GATT_CLIENT
    if (event->hdr.status == ATT_SUCCESS && event->hdr.event == ATT_MTU_UPDATE_IND) {
        ble::BLEInstanceBase &ble = ble::BLEInstanceBase::deviceInstance();
        PalGattClientEventHandler *handler = ble.getPalGattClient().get_event_handler();
        if (handler) {
            handler->on_att_mtu_change(event->hdr.param, event->mtu);
        }
    } else if (event->hdr.event == ATTC_WRITE_CMD_RSP) {
        ble::BLEInstanceBase &ble = ble::BLEInstanceBase::deviceInstance();
        PalGattClientEventHandler *handler = ble.getPalGattClient().get_event_handler();
        if (handler) {
            handler->on_write_command_sent(
                event->hdr.param,
                event->handle,
                event->hdr.status
            );
        }
    } else {
        // all handlers are stored in a static array
        static const event_handler_t handlers[] = {
            &timeout_event_handler,
            &event_handler<ErrorResponseConverter>,
            &event_handler<FindInformationResponseConverter>,
            &event_handler<FindByTypeValueResponseConverter>,
            &event_handler<ReadByTypeResponseConverter>,
            &event_handler<ReadResponseConverter>,
            &event_handler<ReadBlobResponseConverter>,
            &event_handler<ReadMultipleResponseConverter>,
            &event_handler<ReadBygroupTypeResponseConverter>,
            &event_handler<WriteResponseConverter>,
            &event_handler<PrepareWriteResponseConverter>,
            &event_handler<ExecuteWriteResponseConverter>,
            &event_handler<HandleValueIndicationConverter>,
            &event_handler<HandleValueNotificationConverter>
        };

        // event->hdr.param: connection handle
        // event->header.event: opcode from the request
        // event->header.status: success or error code ...
        // event->pValue: starting after opcode for response; starting after opcode + handle for server initiated responses.
        // event->handle: handle for server initiated responses

        // traverse all handlers and execute them with the event in input.
        // exit if an handler has handled the event.
        for (size_t i = 0; i < (sizeof(handlers) / sizeof(handlers[0])); ++i) {
            if (handlers[i](event)) {
                return;
            }
        }
    }
#endif // BLE_FEATURE_GATT_CLIENT

#if BLE_FEATURE_GATT_SERVER
    // pass events not handled to the server side
    ble::GattServer::getInstance().att_cb(event);
#endif // BLE_FEATURE_GATT_SERVER
}

} // ble
