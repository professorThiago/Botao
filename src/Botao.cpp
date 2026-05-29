/**
 * @file Botao.cpp
 * @brief Implementação da biblioteca Botao.
 *
 * @author  professorThiago (https://github.com/professorThiago)
 * @version 1.0.0
 * @license MIT
 */

#include "Botao.h"

// =============================================================================
// Construtor e inicialização
// =============================================================================

Botao::Botao(uint8_t pino, uint8_t modoPino, uint8_t nivelAtivo)
    : _pino(pino), _modoPino(modoPino), _nivelAtivo(nivelAtivo)
{}

void Botao::iniciar()
{
    pinMode(_pino, _modoPino);

    // Lê estado inicial para evitar falso evento na primeira leitura
    _estadoBruto    = (digitalRead(_pino) == _nivelAtivo);
    _estadoAtual    = _estadoBruto;
    _estadoAnterior = _estadoBruto;
    _estadoBrutoAnterior = _estadoBruto;
    _tempoUltimaMudanca  = millis();
}

// =============================================================================
// Configuração de tempos
// =============================================================================

void Botao::intervaloDebounce(uint16_t ms)   { _msDebounce    = ms; }
void Botao::intervaloCliqueDuplo(uint16_t ms){ _msCliqueDuplo = ms; }
void Botao::tempoCliqueLongo(uint16_t ms)    { _msCliqueLongo = ms; }

void Botao::configurarAutoRepeat(uint16_t delayMs, uint16_t intervaloMs)
{
    _msAutoRepeatDelay = delayMs;
    _msAutoRepeatIntv  = intervaloMs;
}

// =============================================================================
// Registro de callbacks
// =============================================================================

void Botao::aoPressionar (CallbackBotao cb) { _cbPressionar  = cb; }
void Botao::aoSoltar     (CallbackBotao cb) { _cbSoltar      = cb; }
void Botao::aoClicar     (CallbackBotao cb) { _cbClicar      = cb; }
void Botao::aoClicarDuplo(CallbackBotao cb) { _cbCliqueDuplo = cb; }
void Botao::aoSegurar    (CallbackBotao cb) { _cbSegurar     = cb; }
void Botao::aoSoltarLongo(CallbackBotao cb) { _cbSoltarLongo = cb; }
void Botao::aoAutoRepeat (CallbackBotao cb) { _cbAutoRepeat  = cb; }

// =============================================================================
// Leitura de flags — retornam true uma vez e limpam o flag
// =============================================================================

bool Botao::pressionou()  { if (!_flagPressionou)  return false; _flagPressionou  = false; return true; }
bool Botao::soltou()      { if (!_flagSoltou)       return false; _flagSoltou      = false; return true; }
bool Botao::clicou()      { if (!_flagClique)       return false; _flagClique      = false; return true; }
bool Botao::clicouDuplo() { if (!_flagCliqueDuplo)  return false; _flagCliqueDuplo = false; return true; }
bool Botao::segurou()     { if (!_flagSegurou)      return false; _flagSegurou     = false; return true; }
bool Botao::soltouLongo() { if (!_flagSoltouLongo)  return false; _flagSoltouLongo = false; return true; }
bool Botao::autoRepetiu() { if (!_flagAutoRepeat)   return false; _flagAutoRepeat  = false; return true; }

// =============================================================================
// Estado atual
// =============================================================================

bool     Botao::estaPresionado()       const { return _estadoAtual;  }
bool     Botao::mudouEstado()          const { return _estadoAtual != _estadoAnterior; }
uint32_t Botao::tempoNoEstadoAtual()   const { return millis() - _tempoUltimaMudanca; }
uint32_t Botao::tempoNoEstadoAnterior()const { return _tempoEstadoAnterior; }
uint32_t Botao::totalCliques()         const { return _totalCliques; }
void     Botao::zerarContador()              { _totalCliques = 0; }
bool     Botao::lerPino()              const { return digitalRead(_pino) == _nivelAtivo; }

// =============================================================================
// Loop principal
// =============================================================================

void Botao::atualizar()
{
    _processarDebounce();
    _processarGestos();
    _processarAutoRepeat();
}

// =============================================================================
// Debounce — intervalo estável
// =============================================================================

void Botao::_processarDebounce()
{
    bool leitura = (digitalRead(_pino) == _nivelAtivo);

    // Reinicia o timer sempre que o sinal bruto muda
    if (leitura != _estadoBrutoAnterior)
    {
        _tempoUltimaMudanca  = millis();
        _estadoBrutoAnterior = leitura;
    }

    // Só aceita a leitura após o intervalo de estabilidade
    if ((millis() - _tempoUltimaMudanca) >= _msDebounce)
    {
        _estadoAnterior = _estadoAtual;
        _estadoAtual    = leitura;
    }
}

// =============================================================================
// Máquina de estados de gestos
// =============================================================================

void Botao::_processarGestos()
{
    bool pressionouAgora = ( _estadoAtual && !_estadoAnterior);
    bool soltouAgora     = (!_estadoAtual &&  _estadoAnterior);
    uint32_t agora       = millis();

    // ── Borda de descida: botão pressionado ──────────────────
    if (pressionouAgora)
    {
        _tempoPresionado = agora;
        _eraCiqueLongo   = false;
        _autoRepeatAtivo = false;

        _dispararEvento(_flagPressionou, _cbPressionar);

        if (_estadoGesto == EstadoGesto::AGUARDANDO_SEGUNDO_CLIQUE)
        {
            // Segundo clique chegou dentro do prazo → duplo clique
            _estadoGesto = EstadoGesto::PRESSIONADO;
            _dispararEvento(_flagCliqueDuplo, _cbCliqueDuplo);
            _totalCliques++;
        }
        else
        {
            _estadoGesto = EstadoGesto::PRESSIONADO;
        }
    }

    // ── Enquanto pressionado: verifica clique longo ──────────
    if (_estadoAtual && _estadoGesto == EstadoGesto::PRESSIONADO)
    {
        uint32_t tempoPreso = agora - _tempoPresionado;

        if (tempoPreso >= _msCliqueLongo && !_eraCiqueLongo)
        {
            _eraCiqueLongo = true;
            _estadoGesto   = EstadoGesto::CLIQUE_LONGO_ATIVO;
            _dispararEvento(_flagSegurou, _cbSegurar);
        }
    }

    // ── Aguardando segundo clique: verifica timeout ──────────
    if (_estadoGesto == EstadoGesto::AGUARDANDO_SEGUNDO_CLIQUE)
    {
        if ((agora - _tempoSolto) >= _msCliqueDuplo)
        {
            // Tempo esgotado → confirma clique simples
            _estadoGesto = EstadoGesto::OCIOSO;
            _dispararEvento(_flagClique, _cbClicar);
            _totalCliques++;
        }
    }

    // ── Borda de subida: botão solto ─────────────────────────
    if (soltouAgora)
    {
        _tempoSolto      = agora;
        _autoRepeatAtivo = false;
        _tempoEstadoAnterior = agora - _tempoPresionado;

        _dispararEvento(_flagSoltou, _cbSoltar);

        if (_estadoGesto == EstadoGesto::CLIQUE_LONGO_ATIVO)
        {
            // Soltou após clique longo
            _estadoGesto = EstadoGesto::OCIOSO;
            _dispararEvento(_flagSoltouLongo, _cbSoltarLongo);
        }
        else if (_estadoGesto == EstadoGesto::PRESSIONADO)
        {
            // Soltou após clique curto — aguarda possível segundo clique
            _estadoGesto = EstadoGesto::AGUARDANDO_SEGUNDO_CLIQUE;
        }
        else if (_estadoGesto == EstadoGesto::AGUARDANDO_SEGUNDO_CLIQUE)
        {
            // Segundo clique detectado no pressionar — já processado
            _estadoGesto = EstadoGesto::OCIOSO;
        }
    }
}

// =============================================================================
// Auto-repeat
// =============================================================================

void Botao::_processarAutoRepeat()
{
    if (!_cbAutoRepeat && !_autoRepeatAtivo) return;
    if (!_estadoAtual) return;

    uint32_t agora      = millis();
    uint32_t tempoPreso = agora - _tempoPresionado;

    if (tempoPreso >= _msAutoRepeatDelay)
    {
        if (!_autoRepeatAtivo)
        {
            _autoRepeatAtivo    = true;
            _tempoUltimoRepeat  = agora;
        }
        else if ((agora - _tempoUltimoRepeat) >= _msAutoRepeatIntv)
        {
            _tempoUltimoRepeat = agora;
            _dispararEvento(_flagAutoRepeat, _cbAutoRepeat);
        }
    }
}

// =============================================================================
// Helper — dispara flag e chama callback
// =============================================================================

void Botao::_dispararEvento(bool& flag, CallbackBotao cb)
{
    flag = true;
    if (cb) cb();
}
