/**
 * @file Botao.h
 * @brief Biblioteca completa para leitura de botões com debounce,
 *        gestos e callbacks para ESP32 e Arduino.
 *
 * @details
 * Substitui a Bounce2 com uma API em português, mais recursos e
 * sem necessidade de verificar eventos manualmente em cada `loop()`.
 *
 * @par Recursos
 * - Debounce em dois modos:
 *   - `ModoDebounce::ESTAVEL`  — integrador: aceita a mudança após N ms (somados) no novo
 *                                nível; glitches curtos não reiniciam a contagem (padrão).
 *   - `ModoDebounce::IMEDIATO` — aceita a 1ª borda na hora e ignora oscilações por N ms
 *                                (resposta instantânea, ideal com `loop()` lento).
 * - Detecção de: clique simples, clique duplo, clique longo, soltura longa.
 * - Auto-repeat: dispara repetidamente enquanto o botão está pressionado.
 * - Callbacks: registre funções para cada evento — sem `if` no `loop()`.
 *   No ESP32/ESP8266/RP2040 aceita lambdas com captura (`std::function`).
 * - Contador de cliques acumulados.
 * - Suporte a botão ativo em LOW (pull-up) ou HIGH (pull-down).
 *
 * @par Uso mínimo (polling)
 * @code
 * #include <Botao.h>
 *
 * Botao btn(5);   // GPIO 5, INPUT_PULLUP, ativo em LOW
 *
 * void setup() { btn.iniciar(); }
 *
 * void loop() {
 *     btn.atualizar();
 *     if (btn.clicou())      Serial.println("Clique!");
 *     if (btn.clicouDuplo()) Serial.println("Duplo!");
 *     if (btn.segurou())     Serial.println("Longo!");
 * }
 * @endcode
 *
 * @par Uso com callbacks
 * @code
 * Botao btn(5);
 *
 * void setup() {
 *     btn.iniciar();
 *     btn.aoClicar([]() { Serial.println("Clique!"); });
 *     btn.aoClicarDuplo([]() { Serial.println("Duplo!"); });
 *     btn.aoSegurar([]() { Serial.println("Longo!"); });
 * }
 *
 * void loop() { btn.atualizar(); }
 * @endcode
 *
 * @author  professorThiago (https://github.com/professorThiago)
 * @version 1.1.0
 * @date    2026
 * @license MIT
 *
 * @par Licença MIT
 * Copyright (c) 2026 professorThiago\n
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:\n
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.\n
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
 */

#ifndef BOTAO_H
#define BOTAO_H

#include <Arduino.h>

// ---------------------------------------------------------------------------
// Tipo de callback
//   ESP32 / ESP8266 / RP2040 → std::function (aceita lambdas com captura)
//   AVR e demais             → ponteiro de função (lambdas sem captura)
// ---------------------------------------------------------------------------
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266) || \
    defined(ARDUINO_ARCH_RP2040) || defined(BOTAO_USAR_STD_FUNCTION)
  #include <functional>
  typedef std::function<void()> CallbackBotao;
#else
  typedef void (*CallbackBotao)();
#endif

/**
 * @brief Estratégia de debounce.
 */
enum class ModoDebounce : uint8_t
{
    ESTAVEL,   ///< Integrador: aceita após `intervaloDebounce` ms acumulados no novo nível.
    IMEDIATO   ///< Aceita a 1ª borda na hora; ignora novas bordas por `intervaloDebounce` ms.
};

/**
 * @brief Gerencia um botão físico com debounce, gestos e callbacks.
 */
class Botao
{
public:
    // -----------------------------------------------------------------------
    // Construtor e inicialização
    // -----------------------------------------------------------------------

    /**
     * @brief Cria um objeto Botao.
     *
     * @param pino         GPIO conectado ao botão.
     * @param modoPino     `INPUT_PULLUP` (padrão), `INPUT` ou `INPUT_PULLDOWN`.
     * @param nivelAtivo   Nível lógico com o botão pressionado.
     *                     `LOW` para pull-up (padrão), `HIGH` para pull-down.
     */
    explicit Botao(uint8_t pino,
                   uint8_t modoPino   = INPUT_PULLUP,
                   uint8_t nivelAtivo = LOW);

    /** @brief Configura o pino e inicializa o estado interno. Chamar no `setup()`. */
    void iniciar();

    /**
     * @brief Lê o pino, aplica debounce e processa todos os eventos.
     *
     * Deve ser chamado **uma vez em todo `loop()`**, o mais frequentemente
     * possível. Evite `delay()` no `loop()`: um toque mais curto que o
     * intervalo entre duas chamadas pode passar despercebido.
     */
    void atualizar();

    // -----------------------------------------------------------------------
    // Configuração
    // -----------------------------------------------------------------------

    /** @brief Intervalo de debounce em ms (padrão: 25). */
    void intervaloDebounce(uint16_t ms);

    /** @brief Estratégia de debounce (padrão: `ModoDebounce::ESTAVEL`). */
    void modoDebounce(ModoDebounce modo);

    /** @brief Tempo máximo entre dois cliques para ser duplo, em ms (padrão: 400). */
    void intervaloCliqueDuplo(uint16_t ms);

    /**
     * @brief Liga/desliga a detecção de clique duplo (padrão: ligada).
     *
     * Com a detecção ligada, `clicou()` só dispara `intervaloCliqueDuplo` ms
     * após soltar (é preciso esperar para saber se virá um 2º clique).
     * Desligue se não usa clique duplo: o clique simples passa a ser imediato.
     */
    void habilitarCliqueDuplo(bool habilitar);

    /** @brief Tempo mínimo pressionado para clique longo, em ms (padrão: 800). */
    void tempoCliqueLongo(uint16_t ms);

    /**
     * @brief Configura e **habilita** o auto-repeat.
     * @param delayMs     Tempo pressionado antes da 1ª repetição (padrão: 600 ms).
     * @param intervaloMs Intervalo entre repetições (padrão: 150 ms).
     */
    void configurarAutoRepeat(uint16_t delayMs = 600, uint16_t intervaloMs = 150);

    /** @brief Desabilita o auto-repeat. */
    void desabilitarAutoRepeat();

    // -----------------------------------------------------------------------
    // Callbacks
    // -----------------------------------------------------------------------

    void aoPressionar (CallbackBotao cb); ///< Borda de pressionar (imediato após debounce).
    void aoSoltar     (CallbackBotao cb); ///< Borda de soltar.
    void aoClicar     (CallbackBotao cb); ///< Clique simples confirmado.
    void aoClicarDuplo(CallbackBotao cb); ///< Clique duplo (dispara no 2º pressionar).
    void aoSegurar    (CallbackBotao cb); ///< Atingiu `tempoCliqueLongo` ainda pressionado.
    void aoSoltarLongo(CallbackBotao cb); ///< Soltou após clique longo.
    void aoAutoRepeat (CallbackBotao cb); ///< Cada repetição (também habilita o auto-repeat).

    // -----------------------------------------------------------------------
    // Polling — retornam true uma única vez e limpam o flag
    // -----------------------------------------------------------------------

    bool pressionou();
    bool soltou();
    bool clicou();
    bool clicouDuplo();
    bool segurou();
    bool soltouLongo();
    bool autoRepetiu();

    // -----------------------------------------------------------------------
    // Estado atual
    // -----------------------------------------------------------------------

    /** @brief `true` enquanto o botão estiver pressionado (após debounce). */
    bool estaPressionado() const;

    /** @deprecated Use `estaPressionado()`. Mantido por compatibilidade. */
    bool estaPresionado() const { return estaPressionado(); }

    /** @brief `true` se o estado mudou no último `atualizar()`. */
    bool mudouEstado() const;

    /** @brief Tempo em ms desde a última mudança de estado (após debounce). */
    uint32_t tempoNoEstadoAtual() const;

    /** @brief Duração em ms do estado anterior (ex.: quanto tempo ficou pressionado). */
    uint32_t tempoNoEstadoAnterior() const;

    /** @brief Total de cliques (simples + duplos) desde a inicialização. */
    uint32_t totalCliques() const;

    /** @brief Zera o contador de cliques. */
    void zerarContador();

    /** @brief Nível lógico bruto do pino, sem debounce (`true` = ativo). */
    bool lerPino() const;

private:
    enum class EstadoGesto : uint8_t
    {
        OCIOSO,
        PRESSIONADO,
        AGUARDANDO_SEGUNDO_CLIQUE,
        SEGUNDO_CLIQUE,          // 2º pressionar de um duplo, aguardando soltar
        CLIQUE_LONGO_ATIVO
    };

    // Configuração
    uint8_t      _pino;
    uint8_t      _modoPino;
    uint8_t      _nivelAtivo;
    ModoDebounce _modoDebounce        = ModoDebounce::ESTAVEL;
    uint16_t     _msDebounce          = 25;
    uint16_t     _msCliqueDuplo       = 400;
    uint16_t     _msCliqueLongo       = 800;
    uint16_t     _msAutoRepeatDelay   = 600;
    uint16_t     _msAutoRepeatIntv    = 150;
    bool         _cliqueDuploHabilitado = true;
    bool         _autoRepeatHabilitado  = false;

    // Debounce
    bool     _estadoAtual           = false;  // true = pressionado
    bool     _estadoAnterior        = false;
    bool     _leituraAnterior       = false;  // última leitura bruta
    uint32_t _tempoMudancaBruta     = 0;      // instante do último atualizar()
    uint32_t _acumulado             = 0;      // integrador do modo ESTAVEL (ms)
    uint32_t _tempoMudancaEstavel   = 0;      // última mudança aceita
    uint32_t _duracaoEstadoAnterior = 0;

    // Gestos
    EstadoGesto _estadoGesto      = EstadoGesto::OCIOSO;
    uint32_t    _tempoPressionado = 0;
    uint32_t    _tempoSolto       = 0;

    // Auto-repeat
    bool     _autoRepeatAtivo   = false;
    uint32_t _tempoUltimoRepeat = 0;

    // Flags de evento
    bool _flagPressionou  = false;
    bool _flagSoltou      = false;
    bool _flagClique      = false;
    bool _flagCliqueDuplo = false;
    bool _flagSegurou     = false;
    bool _flagSoltouLongo = false;
    bool _flagAutoRepeat  = false;

    // Callbacks
    CallbackBotao _cbPressionar  = nullptr;
    CallbackBotao _cbSoltar      = nullptr;
    CallbackBotao _cbClicar      = nullptr;
    CallbackBotao _cbCliqueDuplo = nullptr;
    CallbackBotao _cbSegurar     = nullptr;
    CallbackBotao _cbSoltarLongo = nullptr;
    CallbackBotao _cbAutoRepeat  = nullptr;

    uint32_t _totalCliques = 0;

    // Helpers
    bool _lerBruto() const;
    void _aplicarMudanca(bool novoEstado, uint32_t agora);
    void _processarDebounce(uint32_t agora);
    void _processarGestos(uint32_t agora);
    void _processarAutoRepeat(uint32_t agora);
    void _dispararEvento(bool& flag, const CallbackBotao& cb);
    static bool _consumir(bool& flag);
};

#endif // BOTAO_H
