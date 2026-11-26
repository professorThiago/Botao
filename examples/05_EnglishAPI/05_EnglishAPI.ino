/**
 * @file 05_EnglishAPI.ino
 * @brief Botao — Example 5: the whole library with English names.
 *
 * Every method has an English name that calls the same code as the
 * Portuguese one (begin = iniciar, onClick = aoClicar, clicked = clicou...).
 * `Button` is the same class as `Botao` — both names can be mixed.
 *
 * Wiring: push button between GPIO 5 and GND (internal pull-up).
 *
 * @author  professorThiago (https://github.com/professorThiago)
 * @license MIT
 */

#include <Arduino.h>
#include <Botao.h>

const int LED_PIN = 2;

Button button(5);           // GPIO 5, INPUT_PULLUP, active LOW
int    brightness = 50;     // value adjusted with auto-repeat (0–100)

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    button.begin();

    // ---- Timing (optional) ----
    button.setDebounceTime(25);
    button.setDebounceMode(DebounceMode::STABLE);
    button.setDoubleClickTime(400);
    button.setLongPressTime(800);
    button.setAutoRepeat(600, 150);

    // ---- Callbacks ----
    button.onClick([]() {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        Serial.println("Click -> LED toggled");
    });

    button.onDoubleClick([]() {
        brightness = 50;
        Serial.println("Double click -> brightness reset to 50");
    });

    button.onLongPress([]() {
        Serial.println("Long press started");
    });

    button.onLongRelease([]() {
        Serial.print("Released after ");
        Serial.print(button.previousStateDuration());
        Serial.println(" ms");
    });

    button.onAutoRepeat([]() {
        if (brightness < 100) brightness++;
        Serial.print("Brightness: ");
        Serial.println(brightness);
    });
}

void loop() {
    button.update();   // ALWAYS in loop(), once per pass

    // Polling works too (each event is returned only once)
    if (button.pressed()) {
        Serial.print("Pressed (total clicks so far: ");
        Serial.print(button.clickCount());
        Serial.println(")");
    }
}
