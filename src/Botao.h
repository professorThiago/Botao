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
 * - Debounce por intervalo estável (padrão: 25 ms).
 * - Detecção de: clique simples, clique duplo, clique longo, soltura longa.
 * - Auto-repeat: dispara repetidamente enquanto o botão está pressionado.
 * - Callbacks: registre funções para cada evento — sem `if` no `loop()`.
 * - Contador de cliques acumulados.
 * - Suporte a botão ativo em LOW (pull-up) ou HIGH (pull-down).
 * - Configuração completa via construtor ou métodos encadeados.
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
 *     if (btn.clicou())       Serial.println("Clique!");
 *     if (btn.clicouDuplo()) Serial.println("Duplo!");
 *     if (btn.segurou())      Serial.println("Longo!");
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
 * @version 1.0.0
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
// Tipo de callback — função sem parâmetros e sem retorno
// ---------------------------------------------------------------------------
typedef void (*CallbackBotao)();

// ---------------------------------------------------------------------------
// Classe Botao
// ---------------------------------------------------------------------------

/**
 * @brief Gerencia um botão físico com debounce, gestos e callbacks.
 */
class Botao
{
public:
    // -----------------------------------------------------------------------
    // Construtor
    // -----------------------------------------------------------------------

    /**
     * @brief Cria um objeto Botao.
     *
     * @param pino         GPIO conectado ao botão.
     * @param modoPino     `INPUT_PULLUP` (padrão) ou `INPUT` / `INPUT_PULLDOWN`.
     * @param nivelAtivo   Nível lógico quando o botão está pressionado.
     *                     `LOW` para pull-up (padrão), `HIGH` para pull-down.
     *
     * @par Exemplos
     * @code
     * Botao btn(5);                          // GPIO 5, pull-up interno, ativo LOW
     * Botao btn(5, INPUT_PULLDOWN, HIGH);    // GPIO 5, pull-down, ativo HIGH
     * Botao btn(5, INPUT, LOW);              // GPIO 5, pull-up externo
     * @endcode
     */
    explicit Botao(uint8_t pino,
                   uint8_t modoPino   = INPUT_PULLUP,
                   uint8_t nivelAtivo = LOW);

    // -----------------------------------------------------------------------
    // Inicialização
    // -----------------------------------------------------------------------

    /**
     * @brief Configura o pino e inicializa o estado interno.
     * Deve ser chamado uma vez no `setup()`.
     */
    void iniciar();

    // -----------------------------------------------------------------------
    // Loop principal — DEVE ser chamado em todo loop()
    // -----------------------------------------------------------------------

    /**
     * @brief Lê o pino, aplica debounce e processa todos os eventos.
     *
     * Deve ser chamado **em todo `loop()`**. Dispara os callbacks
     * registrados e atualiza os flags de evento.
     *
     * @note Chame `atualizar()` apenas **uma vez por loop** para cada instância.
     */
    void atualizar();

    // -----------------------------------------------------------------------
    // Configuração de tempos
    // -----------------------------------------------------------------------

    /**
     * @brief Define o intervalo de debounce.
     * @param ms  Tempo em milissegundos (padrão: 25).
     *            Valores menores = mais responsivo mas mais suscetível a ruído.
     *            Valores maiores = mais estável mas menos responsivo.
     */
    void intervaloDebounce(uint16_t ms);

    /**
     * @brief Define o tempo máximo entre dois cliques para ser considerado duplo.
     * @param ms  Tempo em milissegundos (padrão: 400).
     */
    void intervaloCliqueDuplo(uint16_t ms);

    /**
     * @brief Define o tempo mínimo pressionado para ser considerado clique longo.
     * @param ms  Tempo em milissegundos (padrão: 800).
     */
    void tempoCliqueLongo(uint16_t ms);

    /**
     * @brief Configura o auto-repeat — dispara continuamente enquanto pressionado.
     *
     * @param delayMs     Tempo pressionado antes de começar a repetir (padrão: 600 ms).
     * @param intervaloMs Intervalo entre cada repetição (padrão: 150 ms).
     *
     * @par Exemplo
     * @code
     * btn.configurarAutoRepeat(500, 100);   // começa após 500ms, repete a cada 100ms
     * btn.aoAutoRepeat([]() { volume++; });
     * @endcode
     */
    void configurarAutoRepeat(uint16_t delayMs = 600, uint16_t intervaloMs = 150);

    // -----------------------------------------------------------------------
    // Registro de callbacks
    // -----------------------------------------------------------------------

    /**
     * @brief Callback chamado uma vez ao pressionar (borda de descida).
     * @param cb  Função `void cb()`.
     */
    void aoPressionar(CallbackBotao cb);

    /**
     * @brief Callback chamado uma vez ao soltar (borda de subida).
     * @param cb  Função `void cb()`.
     */
    void aoSoltar(CallbackBotao cb);

    /**
     * @brief Callback chamado ao detectar um clique simples.
     *
     * Disparado ao soltar, após confirmar que não é um clique duplo
     * (aguarda `intervaloCliqueDuplo` ms antes de disparar).
     *
     * @param cb  Função `void cb()`.
     */
    void aoClicar(CallbackBotao cb);

    /**
     * @brief Callback chamado ao detectar um clique duplo.
     * @param cb  Função `void cb()`.
     */
    void aoClicarDuplo(CallbackBotao cb);

    /**
     * @brief Callback chamado quando o botão é mantido pressionado além de `tempoCliqueLongo`.
     *
     * Disparado **enquanto** o botão está pressionado, ao atingir o tempo mínimo.
     * Não é necessário soltar para disparar.
     *
     * @param cb  Função `void cb()`.
     */
    void aoSegurar(CallbackBotao cb);

    /**
     * @brief Callback chamado ao soltar após um clique longo.
     * @param cb  Função `void cb()`.
     */
    void aoSoltarLongo(CallbackBotao cb);

    /**
     * @brief Callback chamado repetidamente durante auto-repeat.
     *
     * Requer chamada prévia a `configurarAutoRepeat()`.
     *
     * @param cb  Função `void cb()`.
     */
    void aoAutoRepeat(CallbackBotao cb);

    // -----------------------------------------------------------------------
    // Leitura de estado — polling (alternativa aos callbacks)
    // -----------------------------------------------------------------------

    /**
     * @brief Retorna `true` uma única vez quando o botão é pressionado.
     * O flag é limpo após a leitura.
     */
    bool pressionou();

    /**
     * @brief Retorna `true` uma única vez quando o botão é solto.
     * O flag é limpo após a leitura.
     */
    bool soltou();

    /**
     * @brief Retorna `true` uma única vez ao detectar clique simples.
     * O flag é limpo após a leitura.
     */
    bool clicou();

    /**
     * @brief Retorna `true` uma única vez ao detectar clique duplo.
     * O flag é limpo após a leitura.
     */
    bool clicouDuplo();

    /**
     * @brief Retorna `true` uma única vez quando atinge o tempo de clique longo.
     * O flag é limpo após a leitura.
     */
    bool segurou();

    /**
     * @brief Retorna `true` uma única vez ao soltar após clique longo.
     * O flag é limpo após a leitura.
     */
    bool soltouLongo();

    /**
     * @brief Retorna `true` em cada disparo do auto-repeat.
     * O flag é limpo após a leitura.
     */
    bool autoRepetiu();

    // -----------------------------------------------------------------------
    // Estado atual
    // -----------------------------------------------------------------------

    /** @brief Retorna `true` enquanto o botão estiver pressionado. */
    bool estaPresionado() const;

    /** @brief Retorna `true` se o estado mudou no último `atualizar()`. */
    bool mudouEstado() const;

    /** @brief Retorna o tempo em ms no estado atual (reseta ao mudar). */
    uint32_t tempoNoEstadoAtual() const;

    /** @brief Retorna o tempo em ms que ficou no estado anterior. */
    uint32_t tempoNoEstadoAnterior() const;

    /** @brief Retorna o número total de cliques desde a inicialização. */
    uint32_t totalCliques() const;

    /** @brief Zera o contador de cliques. */
    void zerarContador();

    /** @brief Lê o nível lógico bruto do pino (sem debounce). */
    bool lerPino() const;

private:
    // -----------------------------------------------------------------------
    // Configuração
    // -----------------------------------------------------------------------
    uint8_t  _pino;
    uint8_t  _modoPino;
    uint8_t  _nivelAtivo;

    uint16_t _msDebounce        = 25;
    uint16_t _msCliqueDuplo     = 400;
    uint16_t _msCliqueLongo     = 800;
    uint16_t _msAutoRepeatDelay = 600;
    uint16_t _msAutoRepeatIntv  = 150;

    // -----------------------------------------------------------------------
    // Estado interno de debounce
    // -----------------------------------------------------------------------
    bool     _estadoAtual       = false; // true = pressionado
    bool     _estadoAnterior    = false;
    bool     _estadoBruto       = false;
    bool     _estadoBrutoAnterior = false;
    uint32_t _tempoUltimaMudanca  = 0;
    uint32_t _tempoEstadoAnterior = 0;

    // -----------------------------------------------------------------------
    // Máquina de estados de gestos
    // -----------------------------------------------------------------------
    enum class EstadoGesto : uint8_t {
        OCIOSO,
        PRESSIONADO,
        AGUARDANDO_SEGUNDO_CLIQUE,
        CLIQUE_LONGO_ATIVO
    };

    EstadoGesto _estadoGesto     = EstadoGesto::OCIOSO;
    uint32_t    _tempoPresionado = 0;
    uint32_t    _tempoSolto      = 0;
    bool        _eraCiqueLongo   = false;

    // Auto-repeat
    uint32_t _tempoUltimoRepeat  = 0;
    bool     _autoRepeatAtivo    = false;

    // -----------------------------------------------------------------------
    // Flags de evento (limpos após leitura)
    // -----------------------------------------------------------------------
    bool _flagPressionou   = false;
    bool _flagSoltou       = false;
    bool _flagClique       = false;
    bool _flagCliqueDuplo  = false;
    bool _flagSegurou      = false;
    bool _flagSoltouLongo  = false;
    bool _flagAutoRepeat   = false;

    // -----------------------------------------------------------------------
    // Callbacks registrados
    // -----------------------------------------------------------------------
    CallbackBotao _cbPressionar  = nullptr;
    CallbackBotao _cbSoltar      = nullptr;
    CallbackBotao _cbClicar      = nullptr;
    CallbackBotao _cbCliqueDuplo = nullptr;
    CallbackBotao _cbSegurar     = nullptr;
    CallbackBotao _cbSoltarLongo = nullptr;
    CallbackBotao _cbAutoRepeat  = nullptr;

    // -----------------------------------------------------------------------
    // Contadores
    // -----------------------------------------------------------------------
    uint32_t _totalCliques = 0;

    // -----------------------------------------------------------------------
    // Helpers internos
    // -----------------------------------------------------------------------
    void _processarDebounce();
    void _processarGestos();
    void _processarAutoRepeat();
    void _dispararEvento(bool& flag, CallbackBotao cb);
};

#endif // BOTAO_H
