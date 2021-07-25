/* main.c - Application main entry point */

/* Based on an example from Zephyr toolkit, modified by frank duignan
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/* This example advertises three services:
 * 0x1800 Generic ACCESS (GAP)
 * 0x1801 Generic Attribute (GATT - this is part of the software device and is not used nor is it apparently removable see https://devzone.nordicsemi.com/f/nordic-q-a/15076/removing-the-generic-attribute-0x1801-primary-service-if-the-service-changed-characteristic-is-not-present
 * And a custom service 1-2-3-4-0 
 * This custom service contains a custom characteristic called char_value
 */
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <zephyr.h>

#include <settings/settings.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <device.h>
#include <drivers/sensor.h>
#include <stdio.h>



#define BT_UUID_CUSTOM_SERVICE_VAL BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)0)
static struct bt_uuid_128 my_service_uuid = BT_UUID_INIT_128( BT_UUID_CUSTOM_SERVICE_VAL);
static struct bt_uuid_128 char_id=BT_UUID_INIT_128(BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)5)); // the 128 bit UUID for this gatt value
uint32_t char_value; // the gatt characateristic value that is being shared over BLE	
static ssize_t read_char(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
static ssize_t write_char(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags);

/* The bt_data structure type:
 * {
 * 	uint8_t type : The kind of data encoded in the following structure
 * 	uint8_t data_len : the length of the data encoded
 * 	const uint8_t *data : a pointer to the data
 * }
 * This is used for encoding advertising data
*/
/* The BT_DATA_BYTES macro
 * #define BT_DATA_BYTES(_type, _bytes...) BT_DATA(_type, ((uint8_t []) { _bytes }), sizeof((uint8_t []) { _bytes }))
 * #define BT_DATA(_type, _data, _data_len) \
 *       { \
 *               .type = (_type), \
 *               .data_len = (_data_len), \
 *               .data = (const uint8_t *)(_data), \
 *       }
 * BT_DATA_UUID16_ALL : value indicates that all UUID's are listed in the advertising packet
*/
// bt_data is an array of data structures used in advertising. Each data structure is formatted as described above
static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)), /* specify BLE advertising flags = discoverable, BR/EDR not supported (BLE only) */
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_CUSTOM_SERVICE_VAL /* A 128 Service UUID for the our custom service follows */),
};
	
/*
 * #define BT_GATT_CHARACTERISTIC(_uuid, _props, _perm, _read, _write, _value) 
 * 
 */
BT_GATT_SERVICE_DEFINE(my_service_svc,
	BT_GATT_PRIMARY_SERVICE(&my_service_uuid),
		BT_GATT_CHARACTERISTIC(&char_id.uuid,		
		BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE |  BT_GATT_CHRC_NOTIFY,
		BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
		read_char, write_char, &char_value),
);


struct bt_conn *active_conn=NULL; // use this to maintain a reference to the connection with the central device (if any)


// Callback that is activated when the characteristic is read by central
static ssize_t read_char(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	printf("Got a read %p\n",attr);
	// Could use 'const char *value =  attr->user_data' also here if there is the char value is being maintained with the BLE STACK
	const char *value = (const char *)&char_value; // point at the value in memory
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(char_value)); // pass the value back up through the BLE stack
}
// Callback that is activated when the characteristic is written by central
static ssize_t write_char(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags)
{
	uint8_t *value = attr->user_data;
	printf("Got a write\n");
	memcpy(value, buf, len); // copy the incoming value in the memory occupied by our characateristic variable
	return len;
}
// Callback that is activated when a connection with a central device is established
static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		printk("Connection failed (err 0x%02x)\n", err);
	} else {
		printk("Connected\n");
		active_conn = conn;
	}
}
// Callback that is activated when a connection with a central device is taken down
static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason 0x%02x)\n", reason);
	active_conn = NULL;
}
// structure used to pass connection callback handlers to the BLE stack
static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
};
// This is called when the BLE stack has finished initializing
static void bt_ready(void)
{
	int err;
	printk("Bluetooth initialized\n");

// start advertising see https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/reference/bluetooth/gap.html
/*
 * Excerpt from zephyr/include/bluetooth/bluetooth.h

 * #define BT_LE_ADV_CONN_NAME BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE | \
                                            BT_LE_ADV_OPT_USE_NAME, \
                                            BT_GAP_ADV_FAST_INT_MIN_2, \
                                            BT_GAP_ADV_FAST_INT_MAX_2, NULL)

 Also see : zephyr/include/bluetooth/gap.h for BT_GAP_ADV.... These set the advertising interval to between 100 and 150ms
 
 */
// Start BLE advertising using the ad array defined above
	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}
	printk("Advertising successfully started\n");
}

void main(void)
{
	int err;

	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	bt_ready();
	bt_conn_cb_register(&conn_callbacks);
	printk("Zephyr Microbit V2 minimal BLE example! %s\n", CONFIG_BOARD);			
	while (1) {
		k_sleep(K_SECONDS(1));
		char_value++;
		// int bt_gatt_notify(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *data, u16_t len)
		// conn: Connection object. (NULL for all)
		// attr: Characteristic Value Descriptor attribute.
		// data: Pointer to Attribute data.
		// len: Attribute value length.				
		if (active_conn)
		{
			bt_gatt_notify(active_conn,&my_service_svc.attrs[2], &char_value,sizeof(char_value));			
		}	
	}
}
