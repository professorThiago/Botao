/**
 * @file 04_MultiplosBotoes.ino
 * @brief Botao — Exemplo 4: Múltiplos botões independentes.
 *
 * Cada botão tem seus próprios callbacks e configurações.
 * Escala facilmente para quantos botões forem necessários.
 *
 * Ligação:
 *   Botão A → GPIO 5 e GND
 *   Botão B → GPIO 6 e GND
 *   Botão C → GPIO 7 e GND
 *
 * @author  professorThiago (https://github.com/professorThiago)
 * @license MIT
 */

#include <Arduino.h>
#include "Botao.h"

Botao botaoA(5);
Botao botaoB(6);
Botao botaoC(7);

void setup() {
    Serial.begin(115200);

    // ── Botão A — navegação ──────────────────────────────────
    botaoA.iniciar();
    botaoA.aoClicar([]()      { Serial.println("A: proximo item");    });
    botaoA.aoClicarDuplo([]() { Serial.println("A: primeiro item");   });
    botaoA.aoSegurar([]()     { Serial.println("A: scroll rapido..."); });
    botaoA.configurarAutoRepeat(400, 80);
    botaoA.aoAutoRepeat([]()  { Serial.println("A: scroll...");       });

    // ── Botão B — confirmação ────────────────────────────────
    botaoB.iniciar();
    botaoB.aoClicar([]()      { Serial.println("B: confirmar");       });
    botaoB.aoSegurar([]()     { Serial.println("B: cancelar tudo");   });

    // ── Botão C — ação especial ──────────────────────────────
    botaoC.iniciar();
    botaoC.intervaloCliqueDuplo(300);    // duplo mais rápido para o C
    botaoC.aoClicar([]()      { Serial.println("C: acao 1");          });
    botaoC.aoClicarDuplo([]() { Serial.println("C: acao 2");          });
    botaoC.aoSegurar([]()     { Serial.println("C: acao 3 (longa)");  });
    botaoC.aoSoltarLongo([]() { Serial.printf("C: solto apos %lu ms\n",
                                    botaoC.tempoNoEstadoAnterior());  });

    Serial.println("Tres botoes prontos — A=GPIO5  B=GPIO6  C=GPIO7");
}

void loop() {
    // Atualiza todos os botões — a ordem não importa
    botaoA.atualizar();
    botaoB.atualizar();
    botaoC.atualizar();
}
