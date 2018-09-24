#include <unity.h>
#include <hsb.h>

void test_fixHue_int16_t(void) {
    TEST_ASSERT_EQUAL_INT16(HSB::fixHue_int16_t(100), 100);
    TEST_ASSERT_EQUAL_INT16(HSB::fixHue_int16_t(400), 40);
    TEST_ASSERT_EQUAL_INT16(HSB::fixHue_int16_t(360), 0);
    TEST_ASSERT_EQUAL_INT16(HSB::fixHue_int16_t(0), 0);
    TEST_ASSERT_EQUAL_INT16(HSB::fixHue_int16_t(-40), 320);
}

void test_fixHue(void) {
    TEST_ASSERT_EQUAL_FLOAT(HSB::fixHue(100.0), 100.0);
    TEST_ASSERT_EQUAL_FLOAT(HSB::fixHue(400.0), 40.0);
    TEST_ASSERT_EQUAL_FLOAT(HSB::fixHue(360.0), 0.0);
    TEST_ASSERT_EQUAL_FLOAT(HSB::fixHue(0.0), 0.0);
    TEST_ASSERT_EQUAL_FLOAT(HSB::fixHue(-40.0), 320.0);
}

void test_hueShortestPath(void) {
    TEST_ASSERT_EQUAL_FLOAT(HSB::hueShortestPath(0.0, 90.0), 90.0);
    TEST_ASSERT_EQUAL_FLOAT(HSB::hueShortestPath(340.0, 40.0), 400.0);
    TEST_ASSERT_EQUAL_FLOAT(HSB::hueShortestPath(0.0, 181.0), -179.0);
    TEST_ASSERT_EQUAL_FLOAT(HSB::hueShortestPath(240.0, 0.0), 360);
    TEST_ASSERT_EQUAL_FLOAT(HSB::hueShortestPath(10.0, 340.0), -20.0);
}

void test_HSB_getters(void) {
    HSB hsb(1.0, 2.0, 3.0, 4.0, 5.0);
    TEST_ASSERT_EQUAL_FLOAT(hsb.hue(), 1.0);
    TEST_ASSERT_EQUAL_FLOAT(hsb.saturation(), 2.0);
    TEST_ASSERT_EQUAL_FLOAT(hsb.brightness(), 3.0);
    TEST_ASSERT_EQUAL_FLOAT(hsb.white1(), 4.0);
    TEST_ASSERT_EQUAL_FLOAT(hsb.white2(), 5.0);

    float colors[3];
    hsb.getHSB(colors);
    TEST_ASSERT_EQUAL_FLOAT(colors[0], 1.0);
    TEST_ASSERT_EQUAL_FLOAT(colors[1], 2.0);
    TEST_ASSERT_EQUAL_FLOAT(colors[2], 3.0);
}

void test_constantRgb_brightness_red_100() {
    float colors[3];
    HSB hsb(0.0, 100.0, 100.0, 0.0, 0.0);
    hsb.constantRGB(colors);
    TEST_ASSERT_EQUAL_FLOAT(colors[0], 100.0);
    TEST_ASSERT_EQUAL_FLOAT(colors[1], 0.0);
    TEST_ASSERT_EQUAL_FLOAT(colors[2], 0.0);
}
void test_constantRgb_brightness_green_50() {
    float colors[3];
    HSB hsb(120.0, 100.0, 50.0, 0.0, 0.0);
    hsb.constantRGB(colors);
    TEST_ASSERT_EQUAL_FLOAT(colors[0], 0.0);
    TEST_ASSERT_EQUAL_FLOAT(colors[1], 50.0);
    TEST_ASSERT_EQUAL_FLOAT(colors[2], 0);
}

void test_constantRgb_saturation_50_blue() {
    float colors[3];
    HSB hsb(240.0, 50.0, 100.0, 0.0, 0.0);
    hsb.constantRGB(colors);
    TEST_ASSERT_EQUAL_FLOAT(colors[0], 50.0);
    TEST_ASSERT_EQUAL_FLOAT(colors[1], 50.0);
    TEST_ASSERT_EQUAL_FLOAT(colors[2], 100);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_fixHue_int16_t);
    RUN_TEST(test_fixHue);
    RUN_TEST(test_hueShortestPath);
    RUN_TEST(test_HSB_getters);
    RUN_TEST(test_constantRgb_brightness_green_50);
    RUN_TEST(test_constantRgb_brightness_green_50);
    RUN_TEST(test_constantRgb_saturation_50_blue);
    UNITY_END();

    return 0;
}