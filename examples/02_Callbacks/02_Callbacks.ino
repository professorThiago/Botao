/**
 * @file 02_Callbacks.ino
 * @brief Botao — Exemplo 2: Uso com callbacks e auto-repeat.
 *
 * Com callbacks, o loop() fica limpo — a biblioteca chama
 * suas funções automaticamente quando o evento ocorre.
 *
 * Ligação: botão entre GPIO 5 e GND (pull-up interno).
 *          LED no GPIO 2.
 *
 * @author  professorThiago (https://github.com/professorThiago)
 * @license MIT
 */

#include <Arduino.h>
#include "Botao.h"

#define PINO_BOTAO 5
#define PINO_LED   2

Botao btn(PINO_BOTAO);

bool ledLigado = false;
int  brilho    = 0;

// ── Callbacks ────────────────────────────────────────────────

void aoClicar() {
    ledLigado = !ledLigado;
    digitalWrite(PINO_LED, ledLigado);
    Serial.println(ledLigado ? "LED ligado" : "LED desligado");
}

void aoClicarDuplo() {
    Serial.println("Duplo clique — reiniciando brilho");
    brilho = 0;
}

void aoSegurar() {
    Serial.println("Clique longo — entrando em modo de configuracao");
}

void aoSoltarLongo() {
    Serial.printf("Solto apos %lu ms\n", btn.tempoNoEstadoAnterior());
}

void aoAutoRepeat() {
    brilho = min(255, brilho + 10);
    Serial.printf("Brilho: %d\n", brilho);
}

// ─────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    pinMode(PINO_LED, OUTPUT);

    btn.iniciar();

    // Registra os callbacks
    btn.aoClicar(aoClicar);
    btn.aoClicarDuplo(aoClicarDuplo);
    btn.aoSegurar(aoSegurar);
    btn.aoSoltarLongo(aoSoltarLongo);

    // Auto-repeat: começa após 600ms, repete a cada 100ms
    btn.configurarAutoRepeat(600, 100);
    btn.aoAutoRepeat(aoAutoRepeat);

    Serial.println("Pronto.");
    Serial.println("  Clique simples  → liga/desliga LED");
    Serial.println("  Clique duplo    → reset do brilho");
    Serial.println("  Segurar         → modo configuracao");
    Serial.println("  Segurar+soltar  → mostra tempo pressionado");
    Serial.println("  Segurar longo   → auto-repeat aumenta brilho");
}

void loop() {
    btn.atualizar();   // tudo acontece aqui — loop() fica limpo
}
