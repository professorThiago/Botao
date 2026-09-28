/**
 * @file 01_Basico.ino
 * @brief Botao — Exemplo 1: Uso básico com polling.
 *
 * Demonstra detecção de bordas (pressionou/soltou) e dos gestos
 * clique simples, duplo e longo, sem usar callbacks — estilo polling.
 *
 * Ligação: botão entre GPIO 5 e GND (usa pull-up interno).
 *          Com essa ligação, pressionou() = borda de DESCIDA do pino
 *          e soltou() = borda de SUBIDA.
 *
 * Dicas:
 *  - Não use delay() no loop(): um toque mais curto que o intervalo
 *    entre duas chamadas de atualizar() pode passar despercebido.
 *  - Com clique duplo habilitado, clicou() só dispara 400 ms após soltar
 *    (é preciso esperar para saber se virá um 2º clique). Se não usar
 *    clique duplo, chame btn.habilitarCliqueDuplo(false) e o clique
 *    simples passa a ser imediato.
 *
 * @author  professorThiago (https://github.com/professorThiago)
 * @version 1.1.0
 * @license MIT
 */

#include <Arduino.h>
#include <Botao.h>

Botao btn(5);   // GPIO 5, INPUT_PULLUP, ativo em LOW

void setup() {
    Serial.begin(115200);
    btn.iniciar();

    // Ajuste fino (opcional — os padrões funcionam bem)
    btn.modoDebounce(ModoDebounce::ESTAVEL); // integrador, tolera ruído (padrão)
    btn.intervaloDebounce(25);               // 25 ms no novo nível (padrão)
    btn.intervaloCliqueDuplo(400);           // 400 ms para duplo clique (padrão)
    btn.tempoCliqueLongo(800);               // 800 ms para clique longo (padrão)
    // btn.habilitarCliqueDuplo(false);      // clique simples imediato, sem duplo

    Serial.println("Pressione o botao...");
}

void loop() {
    btn.atualizar();   // SEMPRE no loop(), uma vez por ciclo

    // ── Bordas ───────────────────────────────────────────────
    if (btn.pressionou()) {
        Serial.println("(borda: pressionou)");
    }

    if (btn.soltou()) {
        Serial.printf("(borda: soltou apos %lu ms)\n",
                      (unsigned long)btn.tempoNoEstadoAnterior());
    }

    // ── Gestos ───────────────────────────────────────────────
    if (btn.clicou()) {
        Serial.println("Clique simples!");
        Serial.printf("Total de cliques: %lu\n",
                      (unsigned long)btn.totalCliques());
    }

    if (btn.clicouDuplo()) {
        Serial.println("Clique DUPLO!");
    }

    if (btn.segurou()) {
        Serial.println("Clique LONGO detectado!");
    }

    if (btn.soltouLongo()) {
        Serial.printf("Botao solto apos %lu ms pressionado.\n",
                      (unsigned long)btn.tempoNoEstadoAnterior());
    }
}
