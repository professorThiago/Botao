/**
 * @file 03_GestosAvancados.ino
 * @brief Botao — Exemplo 3: Todos os gestos e informações de tempo.
 *
 * Demonstra como usar tempos para criar comportamentos
 * mais ricos — ex: velocidade proporcional ao tempo pressionado.
 *
 * @author  professorThiago (https://github.com/professorThiago)
 * @license MIT
 */

#include <Arduino.h>
#include "Botao.h"

Botao btn(5);

int valor = 50;   // valor controlado pelo botão (0–100)

void setup() {
    Serial.begin(115200);
    btn.iniciar();

    // Tempos personalizados
    btn.intervaloDebounce(20);
    btn.intervaloCliqueDuplo(350);
    btn.tempoCliqueLongo(1000);
    btn.configurarAutoRepeat(700, 80);

    Serial.println("=== Gestos avancados ===");
    Serial.println("Clique     → +1");
    Serial.println("Duplo      → +10");
    Serial.println("Segurar    → incremento continuo (auto-repeat)");
    Serial.println("Longo+sol  → reset para 50");
}

void loop() {
    btn.atualizar();

    // Clique simples → +1
    if (btn.clicou()) {
        valor = constrain(valor + 1, 0, 100);
        Serial.printf("Valor: %d\n", valor);
    }

    // Clique duplo → +10
    if (btn.clicouDuplo()) {
        valor = constrain(valor + 10, 0, 100);
        Serial.printf("Valor (salto): %d\n", valor);
    }

    // Auto-repeat: incremento contínuo enquanto segura
    if (btn.autoRepetiu()) {
        valor = constrain(valor + 1, 0, 100);
        Serial.printf("Valor (auto): %d\n", valor);
    }

    // Soltar após longo → reset
    if (btn.soltouLongo()) {
        uint32_t tempo = btn.tempoNoEstadoAnterior();
        Serial.printf("Reset! Estava pressionado por %lu ms\n", tempo);
        valor = 50;
    }

    // Exibe tempo atual pressionado (apenas debug)
    if (btn.estaPresionado()) {
        static uint32_t ultimoLog = 0;
        if (millis() - ultimoLog >= 500) {
            ultimoLog = millis();
            Serial.printf("  (pressionado ha %lu ms)\n", btn.tempoNoEstadoAtual());
        }
    }
}
