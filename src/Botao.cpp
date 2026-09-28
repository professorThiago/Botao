/**
 * @file Botao.cpp
 * @brief Implementação da biblioteca Botao.
 *
 * @author  professorThiago (https://github.com/professorThiago)
 * @version 1.2git .0
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

    // Estado inicial = leitura atual → nenhum evento falso na partida
    const bool     leitura = _lerBruto();
    const uint32_t agora   = millis();

    _estadoAtual     = leitura;
    _estadoAnterior  = leitura;
    _leituraAnterior = leitura;

    _tempoMudancaBruta     = agora;
    _tempoMudancaEstavel   = agora;
    _tempoPressionado      = agora;
    _tempoSolto            = agora;
    _duracaoEstadoAnterior = 0;
    _acumulado             = 0;

    _estadoGesto     = EstadoGesto::OCIOSO;
    _autoRepeatAtivo = false;

    _flagPressionou = _flagSoltou = _flagClique = _flagCliqueDuplo =
    _flagSegurou = _flagSoltouLongo = _flagAutoRepeat = false;
}

// =============================================================================
// Configuração
// =============================================================================

void Botao::intervaloDebounce(uint16_t ms)     { _msDebounce    = ms; }
void Botao::modoDebounce(ModoDebounce modo)    { _modoDebounce  = modo; }
void Botao::intervaloCliqueDuplo(uint16_t ms)  { _msCliqueDuplo = ms; }
void Botao::habilitarCliqueDuplo(bool h)       { _cliqueDuploHabilitado = h; }
void Botao::tempoCliqueLongo(uint16_t ms)      { _msCliqueLongo = ms; }

void Botao::configurarAutoRepeat(uint16_t delayMs, uint16_t intervaloMs)
{
    _msAutoRepeatDelay    = delayMs;
    _msAutoRepeatIntv     = intervaloMs;
    _autoRepeatHabilitado = true;
}

void Botao::desabilitarAutoRepeat()
{
    _autoRepeatHabilitado = false;
    _autoRepeatAtivo      = false;
}

// =============================================================================
// Callbacks
// =============================================================================

void Botao::aoPressionar (CallbackBotao cb) { _cbPressionar  = cb; }
void Botao::aoSoltar     (CallbackBotao cb) { _cbSoltar      = cb; }
void Botao::aoClicar     (CallbackBotao cb) { _cbClicar      = cb; }
void Botao::aoClicarDuplo(CallbackBotao cb) { _cbCliqueDuplo = cb; }
void Botao::aoSegurar    (CallbackBotao cb) { _cbSegurar     = cb; }
void Botao::aoSoltarLongo(CallbackBotao cb) { _cbSoltarLongo = cb; }

void Botao::aoAutoRepeat(CallbackBotao cb)
{
    _cbAutoRepeat = cb;
    if (cb) _autoRepeatHabilitado = true;
}

// =============================================================================
// Polling
// =============================================================================

bool Botao::_consumir(bool& flag)
{
    if (!flag) return false;
    flag = false;
    return true;
}

bool Botao::pressionou()  { return _consumir(_flagPressionou);  }
bool Botao::soltou()      { return _consumir(_flagSoltou);      }
bool Botao::clicou()      { return _consumir(_flagClique);      }
bool Botao::clicouDuplo() { return _consumir(_flagCliqueDuplo); }
bool Botao::segurou()     { return _consumir(_flagSegurou);     }
bool Botao::soltouLongo() { return _consumir(_flagSoltouLongo); }
bool Botao::autoRepetiu() { return _consumir(_flagAutoRepeat);  }

// =============================================================================
// Estado atual
// =============================================================================

bool     Botao::estaPressionado()       const { return _estadoAtual; }
bool     Botao::mudouEstado()           const { return _estadoAtual != _estadoAnterior; }
uint32_t Botao::tempoNoEstadoAtual()    const { return millis() - _tempoMudancaEstavel; }
uint32_t Botao::tempoNoEstadoAnterior() const { return _duracaoEstadoAnterior; }
uint32_t Botao::totalCliques()          const { return _totalCliques; }
void     Botao::zerarContador()               { _totalCliques = 0; }
bool     Botao::lerPino()               const { return _lerBruto(); }

bool Botao::_lerBruto() const
{
    return digitalRead(_pino) == _nivelAtivo;
}

// =============================================================================
// Loop principal
// =============================================================================

void Botao::atualizar()
{
    const uint32_t agora = millis();   // um único instante para todo o ciclo
    _processarDebounce(agora);
    _processarGestos(agora);
    _processarAutoRepeat(agora);
}

// =============================================================================
// Debounce
// =============================================================================

void Botao::_aplicarMudanca(bool novoEstado, uint32_t agora)
{
    _duracaoEstadoAnterior = agora - _tempoMudancaEstavel;
    _tempoMudancaEstavel   = agora;
    _estadoAtual           = novoEstado;
}

void Botao::_processarDebounce(uint32_t agora)
{
    // A borda (atual != anterior) deve durar exatamente UM atualizar().
    // Na v1.0 isto só era feito após o intervalo estável — se o sinal
    // oscilasse logo após uma borda, ela era reprocessada várias vezes.
    _estadoAnterior = _estadoAtual;

    const bool leitura = _lerBruto();

    if (_modoDebounce == ModoDebounce::IMEDIATO)
    {
        // Aceita a primeira borda imediatamente; ignora novas bordas
        // até passar o intervalo desde a última mudança aceita.
        if (leitura != _estadoAtual &&
            (agora - _tempoMudancaEstavel) >= _msDebounce)
        {
            _aplicarMudanca(leitura, agora);
        }
        _leituraAnterior = leitura;
        return;
    }

    // ModoDebounce::ESTAVEL — integrador por tempo.
    //
    // A v1.0 exigia `_msDebounce` ms SEM NENHUMA oscilação: um único glitch
    // (ruído do pull-up interno, fio longo, relé/lâmpada chaveando por perto)
    // zerava o timer, e o botão só era aceito depois de muito tempo — ou nunca.
    //
    // Aqui o tempo em que a leitura DIFERE do estado aceito é somado e o tempo
    // em que ela CONCORDA é subtraído. Glitches curtos só descontam a própria
    // duração, em vez de reiniciar tudo. O intervalo real entre chamadas (dt)
    // é usado, então o debounce também funciona com `loop()` lento.
    uint32_t dt = agora - _tempoMudancaBruta;       // tempo desde o último atualizar()
    _tempoMudancaBruta = agora;
    if (dt > _msDebounce) dt = _msDebounce;          // limita o peso de uma única amostra

    if (leitura != _estadoAtual)
    {
        _acumulado += dt;
        if (_acumulado >= _msDebounce)
        {
            _acumulado = 0;
            _aplicarMudanca(leitura, agora);
        }
    }
    else
    {
        _acumulado = (_acumulado > dt) ? (_acumulado - dt) : 0;
    }
    _leituraAnterior = leitura;
}

// =============================================================================
// Máquina de estados de gestos
// =============================================================================

void Botao::_processarGestos(uint32_t agora)
{
    const bool pressionouAgora =  _estadoAtual && !_estadoAnterior;
    const bool soltouAgora     = !_estadoAtual &&  _estadoAnterior;

    // ── Pressionou ───────────────────────────────────────────
    if (pressionouAgora)
    {
        _tempoPressionado = agora;
        _dispararEvento(_flagPressionou, _cbPressionar);

        if (_estadoGesto == EstadoGesto::AGUARDANDO_SEGUNDO_CLIQUE)
        {
            _estadoGesto = EstadoGesto::SEGUNDO_CLIQUE;
            _totalCliques++;
            _dispararEvento(_flagCliqueDuplo, _cbCliqueDuplo);
        }
        else
        {
            _estadoGesto = EstadoGesto::PRESSIONADO;
        }
    }

    // ── Segurando: clique longo ──────────────────────────────
    if (_estadoAtual &&
        _estadoGesto == EstadoGesto::PRESSIONADO &&
        (agora - _tempoPressionado) >= _msCliqueLongo)
    {
        _estadoGesto = EstadoGesto::CLIQUE_LONGO_ATIVO;
        _dispararEvento(_flagSegurou, _cbSegurar);
    }

    // ── Soltou ───────────────────────────────────────────────
    if (soltouAgora)
    {
        _tempoSolto = agora;
        _dispararEvento(_flagSoltou, _cbSoltar);

        switch (_estadoGesto)
        {
            case EstadoGesto::CLIQUE_LONGO_ATIVO:
                _estadoGesto = EstadoGesto::OCIOSO;
                _dispararEvento(_flagSoltouLongo, _cbSoltarLongo);
                break;

            case EstadoGesto::PRESSIONADO:
                if (_cliqueDuploHabilitado)
                {
                    _estadoGesto = EstadoGesto::AGUARDANDO_SEGUNDO_CLIQUE;
                }
                else
                {
                    _estadoGesto = EstadoGesto::OCIOSO;
                    _totalCliques++;
                    _dispararEvento(_flagClique, _cbClicar);
                }
                break;

            default:
                // SEGUNDO_CLIQUE (fim do duplo) ou pressionado desde o boot
                _estadoGesto = EstadoGesto::OCIOSO;
                break;
        }
    }

    // ── Timeout do 2º clique → confirma clique simples ───────
    if (_estadoGesto == EstadoGesto::AGUARDANDO_SEGUNDO_CLIQUE &&
        (agora - _tempoSolto) >= _msCliqueDuplo)
    {
        _estadoGesto = EstadoGesto::OCIOSO;
        _totalCliques++;
        _dispararEvento(_flagClique, _cbClicar);
    }
}

// =============================================================================
// Auto-repeat
// =============================================================================

void Botao::_processarAutoRepeat(uint32_t agora)
{
    if (!_autoRepeatHabilitado || !_estadoAtual)
    {
        _autoRepeatAtivo = false;
        return;
    }

    if (!_autoRepeatAtivo)
    {
        if ((agora - _tempoPressionado) >= _msAutoRepeatDelay)
        {
            _autoRepeatAtivo   = true;
            _tempoUltimoRepeat = agora;
            _dispararEvento(_flagAutoRepeat, _cbAutoRepeat);  // 1ª repetição
        }
    }
    else if ((agora - _tempoUltimoRepeat) >= _msAutoRepeatIntv)
    {
        _tempoUltimoRepeat = agora;
        _dispararEvento(_flagAutoRepeat, _cbAutoRepeat);
    }
}

// =============================================================================
// Helper
// =============================================================================

void Botao::_dispararEvento(bool& flag, const CallbackBotao& cb)
{
    flag = true;
    if (cb) cb();
}
