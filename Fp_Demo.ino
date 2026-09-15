#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>
#include "lvgl_v8_port.h"

#include <Adafruit_Fingerprint.h>

using namespace esp_panel::drivers;
using namespace esp_panel::board;

Board *board = nullptr;

// =====================================================
// R503 UART
// =====================================================

HardwareSerial FPSerial(2);

#define FP_RX 16
#define FP_TX 15

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&FPSerial);

// =====================================================
// GUI
// =====================================================

lv_obj_t *status_label;

// =====================================================
// ENROLL FUNCTION
// =====================================================

void enrollFingerprint(uint8_t id)
{
    int p = -1;

    lv_label_set_text(status_label, "Place finger...");
    lv_refr_now(NULL);

    Serial.println("Place finger...");

    while (p != FINGERPRINT_OK)
    {
        p = finger.getImage();

        if (p == FINGERPRINT_NOFINGER)
        {
            delay(100);
        }
        else if (p != FINGERPRINT_OK)
        {
            Serial.println("Fingerprint read error");
            lv_label_set_text(status_label, "Read error!");
            return;
        }
    }

    Serial.println("Finger detected");

    p = finger.image2Tz(1);

    if (p != FINGERPRINT_OK)
    {
        lv_label_set_text(status_label, "Image error!");
        return;
    }

    lv_label_set_text(status_label, "Remove finger...");
    lv_refr_now(NULL);

    delay(2000);

    while (finger.getImage() != FINGERPRINT_NOFINGER)
    {
        delay(100);
    }

    lv_label_set_text(status_label, "Place same finger...");
    lv_refr_now(NULL);

    p = -1;

    while (p != FINGERPRINT_OK)
    {
        p = finger.getImage();

        if (p == FINGERPRINT_NOFINGER)
        {
            delay(100);
        }
        else if (p != FINGERPRINT_OK)
        {
            lv_label_set_text(status_label, "Read error!");
            return;
        }
    }

    p = finger.image2Tz(2);

    if (p != FINGERPRINT_OK)
    {
        lv_label_set_text(status_label, "Second image error!");
        return;
    }

    lv_label_set_text(status_label, "Creating fingerprint...");
    lv_refr_now(NULL);

    p = finger.createModel();

    if (p != FINGERPRINT_OK)
    {
        lv_label_set_text(status_label, "Finger mismatch!");
        return;
    }

    p = finger.storeModel(id);

    if (p == FINGERPRINT_OK)
    {
        Serial.println("Fingerprint enrolled!");

        lv_label_set_text(status_label,
                           "Fingerprint enrolled!");
    }
    else
    {
        Serial.println("Store failed");

        lv_label_set_text(status_label,
                           "Enrollment failed!");
    }
}

// =====================================================
// BUTTON CALLBACK
// =====================================================

void enroll_button_event(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        Serial.println("ENROLL BUTTON PRESSED");

        // Enroll fingerprint in ID 1
        enrollFingerprint(1);
    }
}

// =====================================================
// GUI
// =====================================================

void create_gui()
{
    lv_obj_t *screen = lv_scr_act();

    lv_obj_set_style_bg_color(
        screen,
        lv_color_hex(0x16181D),
        0
    );

    // Title
    lv_obj_t *title = lv_label_create(screen);

    lv_label_set_text(title, "Fingerprint");
    lv_obj_align(title,
                 LV_ALIGN_TOP_MID,
                 0,
                 40);

    lv_obj_set_style_text_color(
        title,
        lv_color_white(),
        0
    );

    lv_obj_set_style_text_font(
        title,
        &lv_font_montserrat_24,
        0
    );

    // ENROLL BUTTON
    lv_obj_t *btn = lv_btn_create(screen);

    lv_obj_set_size(btn, 300, 80);

    lv_obj_align(btn,
                 LV_ALIGN_CENTER,
                 0,
                 -20);

    lv_obj_add_event_cb(
        btn,
        enroll_button_event,
        LV_EVENT_CLICKED,
        NULL
    );

    lv_obj_t *btn_text = lv_label_create(btn);

    lv_label_set_text(
        btn_text,
        "ENROLL FINGERPRINT"
    );

    lv_obj_center(btn_text);

    // Status
    status_label = lv_label_create(screen);

    lv_label_set_text(
        status_label,
        "Ready"
    );

    lv_obj_align(
        status_label,
        LV_ALIGN_CENTER,
        0,
        80
    );

    lv_obj_set_style_text_color(
        status_label,
        lv_color_hex(0x8BE28B),
        0
    );
}

// =====================================================
// SETUP
// =====================================================

void setup()
{
    Serial.begin(115200);

    // -----------------------------------------------
    // ESP32-S3 LCD
    // -----------------------------------------------

    board = new Board();

    board->init();
    board->begin();

    lvgl_port_init(
        board->getLCD(),
        board->getTouch()
    );

    // -----------------------------------------------
    // R503 UART
    // -----------------------------------------------

    FPSerial.begin(
        57600,
        SERIAL_8N1,
        FP_RX,
        FP_TX
    );

    finger.begin(57600);

    // -----------------------------------------------
    // Check R503
    // -----------------------------------------------

    Serial.println("Checking R503...");

    if (finger.verifyPassword())
    {
        Serial.println("R503 detected");
    }
    else
    {
        Serial.println("R503 NOT detected");
    }

    // -----------------------------------------------
    // GUI
    // -----------------------------------------------

    lvgl_port_lock(-1);

    create_gui();

    lvgl_port_unlock();
}

void loop()
{
    delay(10);
}