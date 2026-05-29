/**
 * @file 01_Basico.ino
 * @brief Botao — Exemplo 1: Uso básico com polling.
 *
 * Demonstra detecção de clique simples, duplo e longo
 * sem usar callbacks — estilo polling tradicional.
 *
 * Ligação: botão entre GPIO 5 e GND (usa pull-up interno).
 *
 * @author  professorThiago (https://github.com/professorThiago)
 * @license MIT
 */

#include <Arduino.h>
#include "Botao.h"

Botao btn(5);   // GPIO 5, INPUT_PULLUP, ativo em LOW

void setup() {
    Serial.begin(115200);
    btn.iniciar();

    // Ajuste fino dos tempos (opcional — os padrões funcionam bem)
    btn.intervaloDebounce(25);      // 25 ms de estabilidade (padrão)
    btn.intervaloCliqueDuplo(400);  // 400 ms para duplo clique (padrão)
    btn.tempoCliqueLongo(800);      // 800 ms para clique longo (padrão)

    Serial.println("Pressione o botao...");
}

void loop() {
    btn.atualizar();   // SEMPRE no loop()

    if (btn.clicou()) {
        Serial.println("Clique simples!");
        Serial.printf("Total de cliques: %lu\n", btn.totalCliques());
    }

    if (btn.clicouDuplo()) {
        Serial.println("Clique DUPLO!");
    }

    if (btn.segurou()) {
        Serial.println("Clique LONGO detectado!");
    }

    if (btn.soltouLongo()) {
        Serial.printf("Botao solto apos %lu ms pressionado.\n",
                      btn.tempoNoEstadoAnterior());
    }

    if (btn.pressionou()) {
        Serial.println("(borda: pressionou)");
    }

    if (btn.soltou()) {
        Serial.println("(borda: soltou)");
    }
}
