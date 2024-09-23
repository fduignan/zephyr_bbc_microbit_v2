/* main.c - Application main entry point */

/* Based on an example from Zephyr toolkit, modified by frank duignan
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/* This example advertises three services:
 * 0x1800 Generic ACCESS (GAP)
 * 0x1801 Generic Attribute (GATT)
 * And a custom service 1-2-3-4-0 
 * This custom service contains a custom characteristic called char_value
 */
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>

#include <zephyr/settings/settings.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>

#include "lsm303_ll.h"
#include "buttons.h"
#include "matrix.h"

struct bt_conn *active_conn=NULL; // use this to maintain a reference to the connection with the central device (if any)


// ********************[ Start of characteristic ]**************************************
#define BT_UUID_X_ACCEL_ID  	   BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)1)
static struct bt_uuid_128 x_accel_id=BT_UUID_INIT_128(BT_UUID_X_ACCEL_ID); // the 128 bit UUID for this gatt value
uint32_t x_accel;
static ssize_t read_x_accel(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
static ssize_t read_x_accel(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	printf("Got a x_accel read %d\n",x_accel);
	const char *value = (const char *)&x_accel;
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(x_accel)); // pass the value back up through the BLE stack
	return 0;
}
// Arguments to BT_GATT_CHARACTERISTIC = _uuid, _props, _perm, _read, _write, _value
#define BT_GATT_ACCELX BT_GATT_CHARACTERISTIC(&x_accel_id.uuid, BT_GATT_CHRC_READ, BT_GATT_PERM_READ, read_x_accel, NULL, &x_accel)
// ********************[ End of characteristic ]**************************************


// ********************[ Service definition ]********************
#define BT_UUID_ACCEL_SERVICE_VAL BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)0)
static struct bt_uuid_128 my_accel_service_uuid = BT_UUID_INIT_128( BT_UUID_ACCEL_SERVICE_VAL);
    
BT_GATT_SERVICE_DEFINE(my_accel_service_svc,
	BT_GATT_PRIMARY_SERVICE(&my_accel_service_uuid),
		BT_GATT_ACCELX		
);


// ********************[ Advertising configuration ]********************
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
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_ACCEL_SERVICE_VAL         /* A 128 Service UUID for the our custom service */),
};

// data structure for the adverised name
static const struct bt_data sd[] = {
        BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};
// Callback that is activated when a connection with a central device is established
static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		printf("Connection failed (err 0x%02x)\n", err);
	} else {
		printf("Connected\n");
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
	printf("Bluetooth initialized\n");

// start advertising see https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/reference/bluetooth/gap.html
/*
 * Excerpt from zephyr/include/bluetooth/bluetooth.h
 * 
 * #define BT_LE_ADV_CONN_NAME BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE | \
                                            BT_LE_ADV_OPT_USE_NAME, \
                                            BT_GAP_ADV_FAST_INT_MIN_2, \
                                            BT_GAP_ADV_FAST_INT_MAX_2, NULL)

 Also see : zephyr/include/bluetooth/gap.h for BT_GAP_ADV.... These set the advertising interval to between 100 and 150ms
 
 */
// Start BLE advertising using the ad array defined above
	err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd , ARRAY_SIZE(sd));
	if (err) {
		printf("Advertising failed to start (err %d)\n", err);
		return;
	}
	printf("Advertising successfully started\n");
}

void main(void)
{
	int err;
    int stepcount_old, btna_old, btnb_old;
    stepcount_old = btna_old = btnb_old = 0;
	err = lsm303_ll_begin();
	if (err < 0)
	{
		 printf("\nError initializing lsm303.  Error code = %d\n",err);  
         while(1);

	}
	err = buttons_begin();
    if (err < 0)
	{
		 printf("\nError initializing buttons.  Error code = %d\n",err);  
         while(1);

	}
	err = matrix_begin();
    if (err < 0)
	{
		 printf("\nError initializing LED matrix.  Error code = %d\n",err);  
         while(1);

	}
	matrix_all_off();
	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}
	bt_ready(); // This function starts advertising
	bt_conn_cb_register(&conn_callbacks);
	printf("Zephyr Microbit V2 demo! %s\n", CONFIG_BOARD);			
	while (1) {
		k_msleep(100);
        x_accel = lsm303_ll_readAccelX();
		
	}
}
