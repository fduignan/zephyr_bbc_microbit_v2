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

// ********************[ Start of characteristic ]**************************************
#define BT_UUID_Y_ACCEL_ID  	   BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)2)
static struct bt_uuid_128 y_accel_id=BT_UUID_INIT_128(BT_UUID_Y_ACCEL_ID); // the 128 bit UUID for this gatt value
uint32_t y_accel;
static ssize_t read_y_accel(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
static ssize_t read_y_accel(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	printf("Got a y_accel read %d\n",y_accel);
	const char *value = (const char *)&y_accel;
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(y_accel)); // pass the value back up through the BLE stack
	return 0;
}
// Arguments to BT_GATT_CHARACTERISTIC = _uuid, _props, _perm, _read, _write, _value
#define BT_GATT_ACCELY BT_GATT_CHARACTERISTIC(&y_accel_id.uuid, BT_GATT_CHRC_READ, BT_GATT_PERM_READ, read_y_accel, NULL, &y_accel)
// ********************[ End of characteristic ]**************************************

// ********************[ Start of characteristic ]**************************************
#define BT_UUID_Z_ACCEL_ID  	   BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)3)
static struct bt_uuid_128 z_accel_id=BT_UUID_INIT_128(BT_UUID_Z_ACCEL_ID); // the 128 bit UUID for this gatt value
uint32_t z_accel;
static ssize_t read_z_accel(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
static ssize_t read_z_accel(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	printf("Got a z_accel read %d\n",z_accel);
	const char *value = (const char *)&z_accel;
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(z_accel)); // pass the value back up through the BLE stack
	return 0;
}
// Arguments to BT_GATT_CHARACTERISTIC = _uuid, _props, _perm, _read, _write, _value
#define BT_GATT_ACCELZ BT_GATT_CHARACTERISTIC(&z_accel_id.uuid, BT_GATT_CHRC_READ, BT_GATT_PERM_READ, read_z_accel, NULL, &z_accel)
// ********************[ End of characteristic ]**************************************


// ********************[ Start of characteristic ]**************************************
#define BT_UUID_STEPCOUNT_ID  	   BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)4)
static struct bt_uuid_128 stepcount_id=BT_UUID_INIT_128(BT_UUID_STEPCOUNT_ID); // the 128 bit UUID for this gatt value
uint32_t stepcount=0;
uint32_t stepcount_subscribed=0;
static ssize_t read_stepcount(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
static ssize_t read_stepcount(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	printf("Got a stepcount read %d\n",stepcount);
	const char *value = (const char *)&stepcount;
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(stepcount)); // pass the value back up through the BLE stack
	return 0;
}
// Callback that is activated when the characteristic is written by central
static ssize_t write_stepcount(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags)
{
	uint8_t *value = attr->user_data;
	printf("Got a stepcount write\n");
	memcpy(value, buf, sizeof(stepcount)); // copy the incoming value in the memory occupied by our characateristic variable
	return len;
}
// Arguments to BT_GATT_CHARACTERISTIC = _uuid, _props, _perm, _read, _write, _value
#define BT_GATT_STEPCOUNT BT_GATT_CHARACTERISTIC(&stepcount_id.uuid,BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE |  BT_GATT_CHRC_NOTIFY | BT_GATT_CCC_NOTIFY, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE, read_stepcount, write_stepcount, &stepcount)
// ********************[ End of First characteristic ]****************************************
static void step_subscribe_changed(const struct bt_gatt_attr *attr,
                                 uint16_t value)
{   if (value==BT_GATT_CCC_NOTIFY)
    {
       printk("Subscribed\n");
       stepcount_subscribed = 1;
    }
    else
    {
        printk("Not Subscribed\n");
        stepcount_subscribed = 0;
    }
}
	
// ********************[ End of characteristic ]**************************************

// ********************[ Service definition ]********************
#define BT_UUID_ACCEL_SERVICE_VAL BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)0)
static struct bt_uuid_128 my_accel_service_uuid = BT_UUID_INIT_128( BT_UUID_ACCEL_SERVICE_VAL);
    
BT_GATT_SERVICE_DEFINE(my_accel_service_svc,
	BT_GATT_PRIMARY_SERVICE(&my_accel_service_uuid),
		BT_GATT_ACCELX,
		BT_GATT_ACCELY,
        BT_GATT_ACCELZ,
        BT_GATT_STEPCOUNT,
        BT_GATT_CCC(step_subscribe_changed,BT_GATT_PERM_READ | BT_GATT_PERM_WRITE)
		
);

// ********************[ Start of characteristic ]**************************************
#define BT_UUID_X_MAG_ID  	   BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)0x1001)
static struct bt_uuid_128 x_mag_id=BT_UUID_INIT_128(BT_UUID_X_MAG_ID); // the 128 bit UUID for this gatt value
uint32_t x_mag;
static ssize_t read_x_mag(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
static ssize_t read_x_mag(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	printf("Got a x_mag read %d\n",x_mag);
	const char *value = (const char *)&x_mag;
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(x_mag)); // pass the value back up through the BLE stack
	return 0;
}
// Arguments to BT_GATT_CHARACTERISTIC = _uuid, _props, _perm, _read, _write, _value
#define BT_GATT_MAGX BT_GATT_CHARACTERISTIC(&x_mag_id.uuid, BT_GATT_CHRC_READ, BT_GATT_PERM_READ, read_x_mag, NULL, &x_mag)
// ********************[ End of characteristic ]**************************************

// ********************[ Start of characteristic ]**************************************
#define BT_UUID_Y_MAG_ID  	   BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)0x1002)
static struct bt_uuid_128 y_mag_id=BT_UUID_INIT_128(BT_UUID_Y_MAG_ID); // the 128 bit UUID for this gatt value
uint32_t y_mag;
static ssize_t read_y_mag(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
static ssize_t read_y_mag(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	printf("Got a y_mag read %d\n",y_mag);
	const char *value = (const char *)&y_mag;
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(y_mag)); // pass the value back up through the BLE stack
	return 0;
}
// Arguments to BT_GATT_CHARACTERISTIC = _uuid, _props, _perm, _read, _write, _value
#define BT_GATT_MAGY BT_GATT_CHARACTERISTIC(&y_mag_id.uuid, BT_GATT_CHRC_READ, BT_GATT_PERM_READ, read_y_mag, NULL, &y_mag)
// ********************[ End of characteristic ]**************************************

// ********************[ Start of characteristic ]**************************************
#define BT_UUID_Z_MAG_ID  	   BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)0x1003)
static struct bt_uuid_128 z_mag_id=BT_UUID_INIT_128(BT_UUID_Z_ACCEL_ID); // the 128 bit UUID for this gatt value
uint32_t z_mag;
static ssize_t read_z_mag(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
static ssize_t read_z_mag(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	printf("Got a z_mag read %d\n",z_mag);
	const char *value = (const char *)&z_mag;
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(z_mag)); // pass the value back up through the BLE stack
	return 0;
}
// Arguments to BT_GATT_CHARACTERISTIC = _uuid, _props, _perm, _read, _write, _value
#define BT_GATT_MAGZ BT_GATT_CHARACTERISTIC(&z_mag_id.uuid, BT_GATT_CHRC_READ, BT_GATT_PERM_READ, read_z_mag, NULL, &z_mag)
// ********************[ End of characteristic ]**************************************

// ********************[ Service definition ]********************
#define BT_UUID_MAG_SERVICE_VAL BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)0x1000)
static struct bt_uuid_128 my_mag_service_uuid = BT_UUID_INIT_128( BT_UUID_MAG_SERVICE_VAL);

BT_GATT_SERVICE_DEFINE(my_mag_service_svc,
	BT_GATT_PRIMARY_SERVICE(&my_mag_service_uuid),
		BT_GATT_MAGX,
		BT_GATT_MAGY,
        BT_GATT_MAGZ
		
);


// ********************[ Start of characteristic ]**************************************
#define BT_UUID_BUTTTON_A_ID  	   BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)0x2001)
static struct bt_uuid_128 button_a_id=BT_UUID_INIT_128(BT_UUID_BUTTTON_A_ID); // the 128 bit UUID for this gatt value
uint32_t button_a;
uint32_t button_a_subscribed;
static ssize_t read_button_a(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
static ssize_t read_button_a(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	printf("Got a button_a read %d\n",button_a);
	const char *value = (const char *)&button_a;
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(button_a)); // pass the value back up through the BLE stack
	return 0;
}
// Arguments to BT_GATT_CHARACTERISTIC = _uuid, _props, _perm, _read, _write, _value
#define BT_GATT_BUTTON_A BT_GATT_CHARACTERISTIC(&button_a_id.uuid, BT_GATT_CHRC_READ |  BT_GATT_CHRC_NOTIFY | BT_GATT_CCC_NOTIFY, BT_GATT_PERM_READ , read_button_a, NULL, &button_a)

static void button_a_subscribe_changed(const struct bt_gatt_attr *attr,
                                 uint16_t value)
{   if (value==BT_GATT_CCC_NOTIFY)
    {
       printk("Subscribed\n");
       button_a_subscribed = 1;
    }
    else
    {
        printk("Not Subscribed\n");
        button_a_subscribed = 0;
    }
}
// ********************[ End of characteristic ]**************************************

#define BT_UUID_BUTTTON_B_ID  	   BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)0x2002)
static struct bt_uuid_128 button_b_id=BT_UUID_INIT_128(BT_UUID_BUTTTON_B_ID); // the 128 bit UUID for this gatt value
uint32_t button_b;
uint32_t button_b_subscribed;
static ssize_t read_button_b(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
static ssize_t read_button_b(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	printf("Got a button_b read %d\n",button_b);
	const char *value = (const char *)&button_b;
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(button_b)); // pass the value back up through the BLE stack
	return 0;
}
// Arguments to BT_GATT_CHARACTERISTIC = _uuid, _props, _perm, _read, _write, _value
#define BT_GATT_BUTTON_B BT_GATT_CHARACTERISTIC(&button_b_id.uuid, BT_GATT_CHRC_READ |  BT_GATT_CHRC_NOTIFY | BT_GATT_CCC_NOTIFY, BT_GATT_PERM_READ , read_button_b, NULL, &button_b)

static void button_b_subscribe_changed(const struct bt_gatt_attr *attr,
                                 uint16_t value)
{   if (value==BT_GATT_CCC_NOTIFY)
    {
       printk("Subscribed\n");
       button_b_subscribed = 1;
    }
    else
    {
        printk("Not Subscribed\n");
        button_b_subscribed = 0;
    }
}

// ********************[ End of characteristic ]**************************************


// ********************[ Service definition ]********************
#define BT_UUID_BUTTON_SERVICE_VAL BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)0x2000)
static struct bt_uuid_128 my_button_service_uuid = BT_UUID_INIT_128( BT_UUID_BUTTON_SERVICE_VAL);
    
BT_GATT_SERVICE_DEFINE(my_button_service_svc,
    BT_GATT_PRIMARY_SERVICE(&my_button_service_uuid),
	BT_GATT_BUTTON_A,
	BT_GATT_CCC(button_a_subscribe_changed,BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
    BT_GATT_BUTTON_B,
   	BT_GATT_CCC(button_b_subscribe_changed,BT_GATT_PERM_READ | BT_GATT_PERM_WRITE)

);



// ********************[ Start of characteristic ]**************************************
#define BT_UUID_LED_ID  	   BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)0x3001)
static struct bt_uuid_128 led_id=BT_UUID_INIT_128(BT_UUID_LED_ID); // the 128 bit UUID for this gatt value
uint32_t led;
static ssize_t read_led(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset);
static ssize_t read_led(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	printf("Got an LED read %d\n",led);
	const char *value = (const char *)&led;
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(led)); // pass the value back up through the BLE stack
	return 0;
}
// Callback that is activated when the characteristic is written by central
static ssize_t write_led(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 const void *buf, uint16_t len, uint16_t offset,
			 uint8_t flags)
{
	uint8_t *value = attr->user_data;
	printf("Got an LED write\n");
	memcpy(value, buf, len); // copy the incoming value in the memory occupied by our characateristic variable
    if (led == 0)
    {
        matrix_all_off();
    }
    else
    {
        matrix_put_pattern(0b11111,~0b11111);
    }
	return len;
}
// Arguments to BT_GATT_CHARACTERISTIC = _uuid, _props, _perm, _read, _write, _value
#define BT_GATT_LED BT_GATT_CHARACTERISTIC(&led_id.uuid, BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE , read_led, write_led, &led)
// ********************[ End of characteristic ]**************************************

// ********************[ Service definition ]********************
#define BT_UUID_LED_SERVICE_VAL BT_UUID_128_ENCODE(1, 2, 3, 4, (uint64_t)0x3000)
static struct bt_uuid_128 my_led_service_uuid = BT_UUID_INIT_128( BT_UUID_LED_SERVICE_VAL);
    
BT_GATT_SERVICE_DEFINE(my_led_service_svc,
    BT_GATT_PRIMARY_SERVICE(&my_led_service_uuid),
    BT_GATT_LED
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
    lsm303_countSteps(&stepcount);
	printf("Zephyr Microbit V2 demo! %s\n", CONFIG_BOARD);			
	while (1) {
		k_msleep(100);
        x_accel = lsm303_ll_readAccelX();
		y_accel = lsm303_ll_readAccelY();
        z_accel = lsm303_ll_readAccelZ();
        x_mag = lsm303_ll_readMagX();
		y_mag = lsm303_ll_readMagY();
        z_mag = lsm303_ll_readMagZ();
        button_a = get_buttonA();
        button_b = get_buttonB();
		// Send a notifiy signal to a central device (if there is one)
		// int bt_gatt_notify(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *data, u16_t len)
		// conn: Connection object. (NULL for all)
		// attr: Characteristic Value Descriptor attribute.
		// data: Pointer to Attribute data.
		// len: Attribute value length.				
		if (active_conn)
		{
            if (stepcount != stepcount_old)
            {
                bt_gatt_notify(active_conn,&my_accel_service_svc.attrs[8], &stepcount,sizeof(stepcount));
                stepcount_old = stepcount;
            }
            if (button_a != btna_old)
            {
                bt_gatt_notify(active_conn,&my_button_service_svc.attrs[2], &button_a,sizeof(button_a));	
                btna_old = button_a;
            }
            if (button_b != btnb_old)
            {
                btnb_old = button_b;
                bt_gatt_notify(active_conn,&my_button_service_svc.attrs[4], &button_b,sizeof(button_b));	
            }
		}	
	}
}
