# Botao

Biblioteca Arduino/ESP32 para leitura de botões com **debounce**, **gestos** e **callbacks**.

> Autor: [professorThiago](https://github.com/professorThiago)

---

## Comparação com a Bounce2

| Recurso | Bounce2 | Botao |
|---------|---------|-------|
| Debounce | ✅ | ✅ |
| Pressionou / Soltou | ✅ | ✅ |
| Clique simples | ✅ | ✅ |
| Clique duplo | ❌ | ✅ |
| Clique longo | ❌ | ✅ |
| Soltura longa | ❌ | ✅ |
| Auto-repeat | ❌ | ✅ |
| Callbacks | ❌ | ✅ |
| Contador de cliques | ❌ | ✅ |
| API em português | ❌ | ✅ |
| Configuração no construtor | ❌ | ✅ |

---

## Instalação

```ini
lib_deps =
    https://github.com/professorThiago/Botao
```

---

## Quick start

### Polling (estilo tradicional)

```cpp
#include <Botao.h>

Botao btn(5);   // GPIO 5, INPUT_PULLUP, ativo em LOW

void setup() { btn.iniciar(); }

void loop() {
    btn.atualizar();

    if (btn.clicou())       Serial.println("Clique!");
    if (btn.clicouDuplo()) Serial.println("Duplo!");
    if (btn.segurou())      Serial.println("Longo!");
}
```

### Callbacks (loop() limpo)

```cpp
#include <Botao.h>

Botao btn(5);

void setup() {
    btn.iniciar();
    btn.aoClicar([]()      { Serial.println("Clique!");  });
    btn.aoClicarDuplo([]() { Serial.println("Duplo!");   });
    btn.aoSegurar([]()     { Serial.println("Longo!");   });
}

void loop() { btn.atualizar(); }
```

---

## Construtor

```cpp
Botao btn(pino);                         // INPUT_PULLUP, ativo em LOW
Botao btn(pino, INPUT_PULLDOWN, HIGH);   // pull-down, ativo em HIGH
Botao btn(pino, INPUT, LOW);             // pull-up externo
```

---

## Configuração de tempos

| Método | Padrão | Descrição |
|--------|--------|-----------|
| `intervaloDebounce(ms)` | 25 ms | Tempo de estabilidade para filtrar ruído |
| `intervaloCliqueDuplo(ms)` | 400 ms | Janela para detectar segundo clique |
| `tempoCliqueLongo(ms)` | 800 ms | Tempo mínimo para clique longo |
| `configurarAutoRepeat(delay, intervalo)` | 600, 150 ms | Início e cadência do auto-repeat |

---

## Eventos — polling

Cada método retorna `true` **uma única vez** por evento e limpa o flag automaticamente.

| Método | Quando dispara |
|--------|---------------|
| `pressionou()` | Borda de descida (ao pressionar) |
| `soltou()` | Borda de subida (ao soltar) |
| `clicou()` | Após confirmar clique simples (não duplo) |
| `clicouDuplo()` | Dois cliques dentro do intervalo configurado |
| `segurou()` | Ao atingir o tempo de clique longo (sem soltar) |
| `soltouLongo()` | Ao soltar após clique longo |
| `autoRepetiu()` | Cada disparo do auto-repeat |

---

## Callbacks

```cpp
btn.aoPressionar(fn);    // borda de descida
btn.aoSoltar(fn);        // borda de subida
btn.aoClicar(fn);        // clique simples confirmado
btn.aoClicarDuplo(fn);   // clique duplo
btn.aoSegurar(fn);       // clique longo atingido
btn.aoSoltarLongo(fn);   // soltou após clique longo
btn.aoAutoRepeat(fn);    // cada disparo do auto-repeat
```

Lambdas são suportadas:

```cpp
btn.aoClicar([]() { digitalWrite(LED, !digitalRead(LED)); });
```

---

## Auto-repeat

Dispara repetidamente enquanto o botão está pressionado — útil para controle de volume, scroll, brilho.

```cpp
btn.configurarAutoRepeat(600, 100);   // começa após 600ms, repete a cada 100ms
btn.aoAutoRepeat([]() { volume++; });
```

---

## Estado e informações

```cpp
btn.estaPresionado()         // true enquanto pressionado
btn.mudouEstado()            // true se mudou no último atualizar()
btn.tempoNoEstadoAtual()     // ms no estado atual
btn.tempoNoEstadoAnterior()  // ms que ficou no estado anterior
btn.totalCliques()           // contador de cliques
btn.zerarContador()          // reseta o contador
btn.lerPino()                // lê o pino bruto (sem debounce)
```

---

## Múltiplos botões

```cpp
Botao botaoA(5);
Botao botaoB(6);
Botao botaoC(7);

void setup() {
    botaoA.iniciar();
    botaoB.iniciar();
    botaoC.iniciar();
    // registre callbacks individualmente
}

void loop() {
    botaoA.atualizar();
    botaoB.atualizar();
    botaoC.atualizar();
}
```

---

## Exemplos

| Exemplo | Descrição |
|---------|-----------|
| `01_Basico` | Polling de clique simples, duplo e longo |
| `02_Callbacks` | Todos os callbacks + auto-repeat |
| `03_GestosAvancados` | Controle de valor com gestos e tempo |
| `04_MultiplosBotoes` | Três botões independentes com callbacks |

---

## Licença

MIT © 2026 [professorThiago](https://github.com/professorThiago)
