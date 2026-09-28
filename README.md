# Botao

Biblioteca Arduino/ESP32 para leitura de botões com **debounce resistente a ruído**, **gestos** (clique, duplo, longo, auto-repeat) e **callbacks** — com API em português.

![PlatformIO Registry](https://badges.registry.platformio.org/packages/professorthiago/library/Botao.svg)
![Licença MIT](https://img.shields.io/badge/licen%C3%A7a-MIT-blue.svg)

> Autor: [professorThiago](https://github.com/professorThiago)

```cpp
Botao btn(5);

void setup() {
    btn.iniciar();
    btn.aoClicar([]()      { Serial.println("Clique!"); });
    btn.aoClicarDuplo([]() { Serial.println("Duplo!");  });
    btn.aoSegurar([]()     { Serial.println("Longo!");  });
}

void loop() { btn.atualizar(); }
```

---

## Sumário

- [Recursos](#recursos)
- [Instalação](#instalação)
- [Quick start](#quick-start)
- [Ligação e construtor](#ligação-e-construtor)
- [Debounce](#debounce)
- [Configuração](#configuração)
- [Eventos (polling)](#eventos-polling)
- [Callbacks](#callbacks)
- [Auto-repeat](#auto-repeat)
- [Estado e informações](#estado-e-informações)
- [Múltiplos botões](#múltiplos-botões)
- [Linha do tempo dos eventos](#linha-do-tempo-dos-eventos)
- [Solução de problemas](#solução-de-problemas)
- [Exemplos](#exemplos)
- [Histórico de versões](#histórico-de-versões)

---

## Recursos

| Recurso | Bounce2 | Botao |
|---------|:-------:|:-----:|
| Debounce | ✅ | ✅ |
| Debounce tolerante a ruído (integrador) | ❌ | ✅ |
| Pressionou / Soltou (bordas) | ✅ | ✅ |
| Duração do estado atual / anterior | ✅ | ✅ |
| Clique simples | ❌ | ✅ |
| Clique duplo | ❌ | ✅ |
| Clique longo e soltura longa | ❌ | ✅ |
| Auto-repeat | ❌ | ✅ |
| Callbacks (inclusive lambdas com captura no ESP32) | ❌ | ✅ |
| Contador de cliques | ❌ | ✅ |
| API em português | ❌ | ✅ |

**Plataformas:** ESP32, ESP8266, RP2040 e AVR (Arduino Uno, Nano, Mega…).

---

## Instalação

### PlatformIO

```ini
; platformio.ini
lib_deps =
    professorthiago/Botao@^1.1.0
```

Ou direto do GitHub:

```ini
lib_deps =
    https://github.com/professorThiago/Botao
```


### Arduino IDE

Baixe o repositório como `.zip` e use **Sketch → Incluir Biblioteca → Adicionar biblioteca .ZIP**.

---

## Quick start

### Polling (estilo tradicional)

```cpp
#include <Arduino.h>
#include <Botao.h>

Botao btn(5);   // GPIO 5, INPUT_PULLUP, ativo em LOW

void setup() {
    Serial.begin(115200);
    btn.iniciar();
}

void loop() {
    btn.atualizar();   // SEMPRE no loop(), uma vez por ciclo

    if (btn.clicou())      Serial.println("Clique!");
    if (btn.clicouDuplo()) Serial.println("Duplo!");
    if (btn.segurou())     Serial.println("Longo!");
}
```

### Callbacks (`loop()` limpo)

```cpp
#include <Arduino.h>
#include <Botao.h>

Botao btn(5);

void setup() {
    Serial.begin(115200);
    btn.iniciar();
    btn.aoClicar([]()      { Serial.println("Clique!"); });
    btn.aoClicarDuplo([]() { Serial.println("Duplo!");  });
    btn.aoSegurar([]()     { Serial.println("Longo!");  });
}

void loop() { btn.atualizar(); }
```

> ⚠️ Evite `delay()` no `loop()`. Um toque mais curto que o intervalo entre duas chamadas de `atualizar()` pode passar despercebido.

---

## Ligação e construtor

```cpp
Botao btn(pino);                         // pull-up interno, ativo em LOW (padrão)
Botao btn(pino, INPUT_PULLDOWN, HIGH);   // pull-down interno, ativo em HIGH (ESP32)
Botao btn(pino, INPUT, LOW);             // pull-up externo, ativo em LOW
Botao btn(pino, INPUT, HIGH);            // pull-down externo, ativo em HIGH
```

A biblioteca trabalha com o estado **lógico** (pressionado/solto), não com a borda física do pino:

| Ligação | `pressionou()` | `soltou()` |
|---------|----------------|------------|
| Botão ao GND + pull-up (`LOW`) | borda de **descida** | borda de subida |
| Botão ao 3V3 + pull-down (`HIGH`) | borda de **subida** | borda de descida |

> Precisa reagir à outra borda sem mudar a ligação? Use `soltou()`. Não inverta o `nivelAtivo` só para isso: `segurou()` e `estaPressionado()` também inverteriam.

---

## Debounce

Dois modos, escolhidos com `modoDebounce(...)`:

| Modo | Como funciona | Quando usar |
|------|---------------|-------------|
| `ModoDebounce::ESTAVEL` *(padrão)* | **Integrador:** soma o tempo em que o pino está no novo nível e desconta o tempo em que volta. Aceita a mudança quando o saldo chega a `intervaloDebounce`. Glitches curtos não reiniciam a contagem. | Quase sempre. Tolera ruído elétrico, fios longos, relés e cargas próximas. |
| `ModoDebounce::IMEDIATO` | Aceita a **primeira** borda na hora e ignora novas bordas por `intervaloDebounce` ms. | Quando a latência zero importa e o sinal é **limpo**. Com ruído, gera eventos falsos. |

```cpp
btn.modoDebounce(ModoDebounce::IMEDIATO);
btn.intervaloDebounce(30);
```

---

## Configuração

| Método | Padrão | Descrição |
|--------|--------|-----------|
| `intervaloDebounce(ms)` | 25 ms | Tempo (acumulado) no novo nível para aceitar a mudança |
| `modoDebounce(modo)` | `ESTAVEL` | Estratégia de debounce (ver acima) |
| `intervaloCliqueDuplo(ms)` | 400 ms | Janela para detectar o segundo clique |
| `habilitarCliqueDuplo(bool)` | `true` | Com `false`, `clicou()` dispara **imediatamente** ao soltar |
| `tempoCliqueLongo(ms)` | 800 ms | Tempo mínimo pressionado para clique longo |
| `configurarAutoRepeat(delay, intervalo)` | 600, 150 ms | Habilita o auto-repeat e define início e cadência |
| `desabilitarAutoRepeat()` | — | Desliga o auto-repeat |

> 💡 **Clique simples lento?** Com clique duplo habilitado, `clicou()` só dispara `intervaloCliqueDuplo` ms depois de soltar, porque a biblioteca precisa esperar para saber se virá um 2º clique. Se você não usa clique duplo, chame `habilitarCliqueDuplo(false)`. Para reagir na hora, use `pressionou()`.

---

## Eventos (polling)

Cada método retorna `true` **uma única vez** por evento e limpa o flag automaticamente.

| Método | Quando dispara |
|--------|----------------|
| `pressionou()` | Ao pressionar (após o debounce) |
| `soltou()` | Ao soltar |
| `clicou()` | Clique simples confirmado (não foi duplo nem longo) |
| `clicouDuplo()` | No 2º pressionar, dentro de `intervaloCliqueDuplo` |
| `segurou()` | Ao atingir `tempoCliqueLongo`, ainda pressionado |
| `soltouLongo()` | Ao soltar após um clique longo |
| `autoRepetiu()` | Em cada disparo do auto-repeat |

---

## Callbacks

```cpp
btn.aoPressionar(fn);    // ao pressionar
btn.aoSoltar(fn);        // ao soltar
btn.aoClicar(fn);        // clique simples confirmado
btn.aoClicarDuplo(fn);   // clique duplo
btn.aoSegurar(fn);       // clique longo atingido
btn.aoSoltarLongo(fn);   // soltou após clique longo
btn.aoAutoRepeat(fn);    // cada disparo do auto-repeat (também habilita o auto-repeat)
```

Funções comuns e lambdas são aceitas:

```cpp
btn.aoClicar([]() { digitalWrite(LED, !digitalRead(LED)); });
```

**Lambdas com captura** (`[this]`, `[&valor]`…) funcionam em **ESP32, ESP8266 e RP2040**, onde o callback é um `std::function`. No AVR (Uno/Nano/Mega) o callback é um ponteiro de função, então use lambdas **sem captura** ou funções comuns.

```cpp
// ESP32 — dentro de uma classe
btn.aoClicar([this]() { alternarLampada(); });
```

---

## Auto-repeat

Dispara repetidamente enquanto o botão está pressionado. Útil para volume, brilho, scroll ou ajuste de valores.

```cpp
btn.configurarAutoRepeat(600, 100);   // 1º disparo após 600 ms, depois a cada 100 ms
btn.aoAutoRepeat([]() { volume++; });

// ou via polling:
if (btn.autoRepetiu()) volume++;
```

---

## Estado e informações

```cpp
btn.estaPressionado()        // true enquanto pressionado (após debounce)
btn.mudouEstado()            // true se o estado mudou no último atualizar()
btn.tempoNoEstadoAtual()     // ms desde a última mudança de estado
btn.tempoNoEstadoAnterior()  // ms no estado anterior (ex.: quanto tempo ficou pressionado)
btn.totalCliques()           // cliques simples + duplos desde o início
btn.zerarContador()          // zera o contador
btn.lerPino()                // nível bruto do pino, sem debounce (true = ativo)
```

> `estaPresionado()` (grafia antiga) continua funcionando, mas está obsoleto.

---

## Múltiplos botões

Cada instância é independente:

```cpp
Botao botaoA(5);
Botao botaoB(6);
Botao botaoC(7);

void setup() {
    botaoA.iniciar();
    botaoB.iniciar();
    botaoC.iniciar();

    botaoA.aoClicar([]() { Serial.println("A"); });
    botaoB.aoClicar([]() { Serial.println("B"); });
    botaoC.aoSegurar([]() { Serial.println("C longo"); });
}

void loop() {
    botaoA.atualizar();
    botaoB.atualizar();
    botaoC.atualizar();
}
```

---

## Linha do tempo dos eventos

Com os tempos padrão (debounce 25 ms, duplo 400 ms, longo 800 ms):

```
Clique simples
  aperta ──[25ms]── pressionou ── solta ──[25ms]── soltou ──[400ms]── clicou

Clique duplo
  pressionou → soltou → pressionou + clicouDuplo → soltou        (sem clicou)

Clique longo
  pressionou ──[800ms segurando]── segurou ── solta ── soltou + soltouLongo   (sem clicou)

Auto-repeat (600, 150)
  pressionou ──[600ms]── autoRepetiu ──[150ms]── autoRepetiu ── ...
```

---

## Solução de problemas

**Preciso segurar o botão muito tempo para ele responder**
- Atualize para a v1.1 ou superior. O debounce antigo reiniciava a cada glitch de ruído.
- Remova `delay()` do `loop()` e chame `atualizar()` o mais frequentemente possível.
- Se você usa `clicou()`, lembre da espera do clique duplo (veja [Configuração](#configuração)).

**Cliques falsos ou fantasmas**
- Use o modo `ESTAVEL` (padrão). O `IMEDIATO` não filtra ruído.
- Aumente `intervaloDebounce` (ex.: 40–50 ms).
- No hardware: o pull-up interno do ESP32 é fraco (~45 kΩ). Com fios longos ou cargas por perto, use um **pull-up externo de 10 kΩ** e um **capacitor de 100 nF** entre o pino e o GND.

**`pressionou()` dispara ao soltar**
- O `nivelAtivo` do construtor não corresponde à ligação. Veja [Ligação e construtor](#ligação-e-construtor).

**Um evento aparece duas vezes**
- `atualizar()` está sendo chamado mais de uma vez por ciclo, ou os flags estão sendo lidos em dois lugares. Cada flag é consumido na primeira leitura.

---

## Exemplos

| Exemplo | Descrição |
|---------|-----------|
| `01_Basico` | Polling de bordas e de clique simples, duplo e longo |
| `02_Callbacks` | Todos os callbacks + auto-repeat |
| `03_GestosAvancados` | Controle de valor com gestos e tempo |
| `04_MultiplosBotoes` | Três botões independentes com callbacks |

---

## Histórico de versões

### 1.1.0
- **Debounce reescrito (integrador):** glitches de ruído não reiniciam mais a contagem, e o botão responde mesmo com `loop()` lento.
- Novo `modoDebounce()` com o modo `IMEDIATO` (latência zero).
- Novo `habilitarCliqueDuplo(false)` para clique simples imediato.
- Novo `desabilitarAutoRepeat()`.
- Callbacks aceitam lambdas com captura no ESP32/ESP8266/RP2040.
- `estaPressionado()` com grafia corrigida (a antiga continua válida).
- **Correções:**
  - `pressionou()` podia disparar duas vezes quando o sinal oscilava logo após a borda.
  - Clique duplo gerava também um clique simples.
  - Auto-repeat não funcionava só com polling, e a 1ª repetição era pulada.
  - `tempoNoEstadoAtual()` zerava com ruído.

### 1.0.0
- Versão inicial.

---

## Licença

MIT © 2026 [professorThiago](https://github.com/professorThiago)
