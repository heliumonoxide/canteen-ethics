/* Edge Impulse Arduino examples
 * Copyright (c) 2022 EdgeImpulse Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction...
 * [License text truncated for brevity]
 */

// These sketches are tested with 2.0.4 ESP32 Arduino Core
// https://github.com/espressif/arduino-esp32/releases/tag/2.0.4

/* Includes ---------------------------------------------------------------- */
#include <plate-person-detection_v2_inferencing.h>
#include "edge-impulse-sdk/dsp/image/image.hpp"

#include "esp_camera.h"
#include <ctype.h> // For tolower() if you choose to use case-insensitive comparison

// Select camera model
#define CAMERA_MODEL_AI_THINKER // Has PSRAM

#if defined(CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#else
#error "Camera model not selected"
#endif

/* Constant defines -------------------------------------------------------- */
#define EI_CAMERA_RAW_FRAME_BUFFER_COLS           320
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS           240
#define EI_CAMERA_FRAME_BYTE_SIZE                 3

/* Private variables ------------------------------------------------------- */
static bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal
static bool is_initialised = false;
uint8_t *snapshot_buf; // Points to the output of the capture

static camera_config_t camera_config = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sscb_sda = SIOD_GPIO_NUM,
    .pin_sscb_scl = SIOC_GPIO_NUM,

    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,

    // XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG, // YUV422, GRAYSCALE, RGB565, JPEG
    .frame_size = FRAMESIZE_QVGA,    // QQVGA-UXGA Do not use sizes above QVGA when not JPEG

    .jpeg_quality = 12, // 0-63 lower number means higher quality
    .fb_count = 1,       // If more than one, i2s runs in continuous mode. Use only with JPEG
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

/* Function definitions ------------------------------------------------------- */
bool ei_camera_init(void);
void ei_camera_deinit(void);
bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf);
static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr);

/* Global variables for detection and violation logic ------------------------- */
float plate_confidence_sum = 0.0;
int plate_detection_count = 0;
float human_confidence_sum = 0.0;
int human_detection_count = 0;
int image_count = 0;

int data_count = 0; // Counts number of DATA (10 images)
int violation_count = 0; // Total number of violations
int consecutive_no_human_data_count = 0; // Counts consecutive DATA with plates but no humans
bool counting_violation = false; // Flag to indicate counting of potential violation

// Timing variables
unsigned long last_firestore_send_time = 0;
const unsigned long FIRESTORE_SEND_INTERVAL = 10 * 60 * 1000; // 10 minutes in milliseconds

// Removed Wi-Fi and Firestore variables as they are not needed for testing

/**
 * @brief      Arduino setup function
 */
void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    // comment out the below line to start inference immediately after upload
    while (!Serial);
    Serial.println("Edge Impulse Inferencing Demo with Violation Detection");

    if (ei_camera_init() == false) {
        ei_printf("Failed to initialize Camera!\r\n");
    }
    else {
        ei_printf("Camera initialized\r\n");
    }

    ei_printf("\nStarting continuous inference in 2 seconds...\n");
    ei_sleep(500);
}

/**
 * @brief      Get data and run inferencing
 *
 * @param[in]  debug  Get debug info if true
 */
void loop()
{
    // Capture image every 2 seconds
    if (ei_sleep(2000) != EI_IMPULSE_OK) {
        return;
    }

    snapshot_buf = (uint8_t*)malloc(EI_CAMERA_RAW_FRAME_BUFFER_COLS * EI_CAMERA_RAW_FRAME_BUFFER_ROWS * EI_CAMERA_FRAME_BYTE_SIZE);

    // Check if allocation was successful
    if(snapshot_buf == nullptr) {
        ei_printf("ERR: Failed to allocate snapshot buffer!\n");
        return;
    }

    ei::signal_t signal;
    signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
    signal.get_data = &ei_camera_get_data;

    if (ei_camera_capture((size_t)EI_CLASSIFIER_INPUT_WIDTH, (size_t)EI_CLASSIFIER_INPUT_HEIGHT, snapshot_buf) == false) {
        ei_printf("Failed to capture image\r\n");
        free(snapshot_buf);
        return;
    }

    // Run the classifier
    ei_impulse_result_t result = { 0 };

    EI_IMPULSE_ERROR err = run_classifier(&signal, &result, debug_nn);
    if (err != EI_IMPULSE_OK) {
        ei_printf("ERR: Failed to run classifier (%d)\n", err);
        free(snapshot_buf);
        return;
    }

    // Variables to accumulate confidences for this image
    float plate_confidence_image = 0.0;
    float human_confidence_image = 0.0;
    int plate_count_image = 0;
    int human_count_image = 0;

    // Process the detection results
#if EI_CLASSIFIER_OBJECT_DETECTION == 1
    // Object detection bounding boxes
    for (uint32_t i = 0; i < result.bounding_boxes_count; i++) {
        ei_impulse_result_bounding_box_t bb = result.bounding_boxes[i];
        if (bb.value == 0) {
            continue;
        }
        ei_printf("Detected '%s' (Confidence: %f) at [ x: %u, y: %u, width: %u, height: %u ]\r\n",
                bb.label,
                bb.value,
                bb.x,
                bb.y,
                bb.width,
                bb.height);

        // Adjusted label comparisons to match exact case
        if (strcmp(bb.label, "Plate") == 0) { // Changed to "Plate"
            plate_confidence_image += bb.value;
            plate_count_image++;
        }
        else if (strcmp(bb.label, "Human") == 0) { // Changed to "Human"
            human_confidence_image += bb.value;
            human_count_image++;
        }

        // Optional: For case-insensitive comparison, uncomment the following and comment out the above if-else
        /*
        if (compareIgnoreCase(bb.label, "plate")) {
            plate_confidence_image += bb.value;
            plate_count_image++;
        }
        else if (compareIgnoreCase(bb.label, "human")) {
            human_confidence_image += bb.value;
            human_count_image++;
        }
        */
    }
#endif

    // Accumulate confidences and counts
    if (plate_count_image > 0) {
        plate_confidence_sum += (plate_confidence_image / plate_count_image); // Average confidence for this image
        plate_detection_count++;
    }
    if (human_count_image > 0) {
        human_confidence_sum += (human_confidence_image / human_count_image); // Average confidence for this image
        human_detection_count++;
    }

    image_count++;

    // Free snapshot buffer
    free(snapshot_buf);

    // After 10 images (1 DATA), compute averages
    if (image_count >= 10) {
        data_count++;

        float avg_plate_confidence = (plate_detection_count > 0) ? (plate_confidence_sum / plate_detection_count) : 0.0;
        float avg_human_confidence = (human_detection_count > 0) ? (human_confidence_sum / human_detection_count) : 0.0;

        ei_printf("Data #%d: Avg Plate Confidence: %.3f, Avg Human Confidence: %.3f\r\n", data_count, avg_plate_confidence, avg_human_confidence);

        // Violation logic
        if (plate_detection_count > 0 && human_detection_count == 0) {
            // Plates detected but no humans
            consecutive_no_human_data_count++;
            ei_printf("Consecutive DATA with plates but no humans: %d\r\n", consecutive_no_human_data_count);

            if (consecutive_no_human_data_count >= 30) {
                // Violation detected
                violation_count++;
                ei_printf("Violation detected! Total violations: %d\r\n", violation_count);
                consecutive_no_human_data_count = 0; // Reset counter
            }
        }
        else if (human_detection_count > 0) {
            // Reset counter if human detected
            consecutive_no_human_data_count = 0;
        }

        // Reset counters for next DATA
        plate_confidence_sum = 0.0;
        plate_detection_count = 0;
        human_confidence_sum = 0.0;
        human_detection_count = 0;
        image_count = 0;
    }

    // Since Firestore functions are disabled, we won't send data to Firestore
    // You can monitor the serial output for testing the image recognition

    // Uncomment the following lines if you want to print violation count periodically
    /*
    unsigned long current_time = millis();
    if (current_time - last_firestore_send_time >= FIRESTORE_SEND_INTERVAL) {
        last_firestore_send_time = current_time;
        ei_printf("Total violations in the last 10 minutes: %d\r\n", violation_count);
        // Reset violation count after reporting
        violation_count = 0;
    }
    */
}

/**
 * @brief   Setup image sensor & start streaming
 *
 * @retval  false if initialization failed
 */
bool ei_camera_init(void) {

    if (is_initialised) return true;

    // Initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }

    sensor_t * s = esp_camera_sensor_get();
    // Initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1); // Flip it back
        s->set_brightness(s, 1); // Up the brightness just a bit
        s->set_saturation(s, 0); // Lower the saturation
    }

    is_initialised = true;
    return true;
}

/**
 * @brief      Stop streaming of sensor data
 */
void ei_camera_deinit(void) {

    // Deinitialize the camera
    esp_err_t err = esp_camera_deinit();

    if (err != ESP_OK)
    {
        ei_printf("Camera deinit failed\n");
        return;
    }

    is_initialised = false;
    return;
}


/**
 * @brief      Capture, rescale and crop image
 *
 * @param[in]  img_width     width of output image
 * @param[in]  img_height    height of output image
 * @param[in]  out_buf       pointer to store output image
 *
 * @retval     false if not initialized, image captured, rescaled or cropped failed
 *
 */
bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf) {
    bool do_resize = false;

    if (!is_initialised) {
        ei_printf("ERR: Camera is not initialized\r\n");
        return false;
    }

    camera_fb_t *fb = esp_camera_fb_get();

    if (!fb) {
        ei_printf("Camera capture failed\n");
        return false;
    }

    bool converted = fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, snapshot_buf);

    esp_camera_fb_return(fb);

    if(!converted){
        ei_printf("Conversion failed\n");
        return false;
    }

    if ((img_width != EI_CAMERA_RAW_FRAME_BUFFER_COLS)
        || (img_height != EI_CAMERA_RAW_FRAME_BUFFER_ROWS)) {
        do_resize = true;
    }

    if (do_resize) {
        ei::image::processing::crop_and_interpolate_rgb888(
        out_buf,
        EI_CAMERA_RAW_FRAME_BUFFER_COLS,
        EI_CAMERA_RAW_FRAME_BUFFER_ROWS,
        out_buf,
        img_width,
        img_height);
    }

    return true;
}

static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr)
{
    // We already have an RGB888 buffer, so recalculate offset into pixel index
    size_t pixel_ix = offset * 3;
    size_t pixels_left = length;
    size_t out_ptr_ix = 0;

    while (pixels_left != 0) {
        // Swap BGR to RGB here
        // due to https://github.com/espressif/esp32-camera/issues/379
        out_ptr[out_ptr_ix] = (snapshot_buf[pixel_ix + 2] << 16) + (snapshot_buf[pixel_ix + 1] << 8) + snapshot_buf[pixel_ix];

        // Go to the next pixel
        out_ptr_ix++;
        pixel_ix+=3;
        pixels_left--;
    }
    // And done!
    return 0;
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_CAMERA
#error "Invalid model for current sensor"
#endif

/* Optional: Function for case-insensitive comparison
bool compareIgnoreCase(const char *str1, const char *str2) {
    while (*str1 && *str2) {
        if (tolower(*str1) != tolower(*str2)) {
            return false;
        }
        str1++;
        str2++;
    }
    return *str1 == *str2;
}
*/
