# Controle de Matriz WS2812 com Interrupções no Raspberry Pi Pico

Este projeto utiliza um **Raspberry Pi Pico** para controlar uma **matriz 5x5 de LEDs WS2812** (NeoPixel), exibindo números de 0 a 9.  
O projeto também inclui **botões** para alterar o número mostrado e um **LED RGB** piscando a 5Hz.

## 📌 **Funcionalidades**
✅ Exibe números de 0 a 9 em uma matriz 5x5 de LEDs WS2812.  
✅ Botão **A** (GPIO 5) **incrementa** o número.  
✅ Botão **B** (GPIO 6) **decrementa** o número.  
✅ LED vermelho (GPIO 13) pisca **5 vezes por segundo** (5 Hz).  
✅ Utiliza **interrupções (IRQ)** para os botões.  
✅ Implementa **debouncing** por software para evitar leituras duplicadas.  

---

## 🎥 **Demonstração**
Confira o funcionamento do projeto neste vídeo:
[![Vídeo de demonstração](https://img.youtube.com/vi/zd_Ih7RQxVw/0.jpg)](https://youtu.be/zd_Ih7RQxVw)

---

## 🛠 **Componentes Necessários**
- 🖥 **Raspberry Pi Pico**
- 🟢 **Matriz WS2812 (NeoPixel) 5x5** (conectada ao **GPIO 7**)
- 🛑 **LED RGB** (usado apenas o vermelho no **GPIO 13**)
- 🔘 **Botão A** (**GPIO 5**, conectado a **GND**)
- 🔘 **Botão B** (**GPIO 6**, conectado a **GND**)
- 📏 **Resistores de pull-up internos ativados via software**
- 🔌 **Fonte de alimentação 5V para os WS2812**

---

## 🔌 **Esquema de Conexão**
| Componente | Pino no Pico |
|------------|-------------|
| Matriz WS2812 (DIN) | GPIO 7 |
| LED RGB (Vermelho) | GPIO 13 |
| Botão A | GPIO 5 (ligado a GND) |
| Botão B | GPIO 6 (ligado a GND) |
| VCC (WS2812) | 5V |
| GND (WS2812 e Botões) | GND |

**Importante:** O fio **GND** dos botões **deve estar conectado** ao GND do Pico para que as interrupções funcionem corretamente!

---

## ⚙ **Configuração do Ambiente**
Antes de compilar e rodar o código no Raspberry Pi Pico, configure o ambiente de desenvolvimento:

### **1️⃣ Instalar o SDK do Raspberry Pi Pico**
Se ainda não tiver o SDK do Pico configurado, siga as instruções [aqui](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf).

No Linux:
```sh
git clone https://github.com/raspberrypi/pico-sdk.git --branch master
cd pico-sdk
git submodule update --init
export PICO_SDK_PATH=$(pwd)
```

No Windows (PowerShell):
```powershell
git clone https://github.com/raspberrypi/pico-sdk.git --branch master
cd pico-sdk
git submodule update --init
$env:PICO_SDK_PATH = Get-Location
```

---

### **2️⃣ Compilar o Projeto**
1. **Clone este repositório**:
   ```sh
   git clone https://github.com/seu-usuario/seu-repositorio.git
   cd seu-repositorio
   ```
2. **Criar a pasta de build**:
   ```sh
   mkdir build
   cd build
   ```
3. **Gerar os arquivos de build**:
   ```sh
   cmake ..
   ```
4. **Compilar o projeto**:
   ```sh
   make
   ```
   Isso gerará um arquivo **`.uf2`** pronto para ser carregado no Raspberry Pi Pico.

---

### **3️⃣ Gravar no Raspberry Pi Pico**
1. Conecte o **Pico** ao computador segurando o botão **BOOTSEL**.
2. Ele será detectado como um **dispositivo USB**.
3. Arraste e solte o arquivo `.uf2` gerado em `/build/`.
4. O Pico irá **reiniciar automaticamente** e executar o código!

---

## 🛠 **Como o Código Funciona**
### **💡 Interrupções nos Botões**
- O código usa **GPIO IRQs** para os botões.
- A função **`gpio_set_irq_enabled_with_callback()`** é usada para configurar um **único callback** global para múltiplos pinos.
- A função `gpio_callback()` identifica **qual pino foi pressionado** e altera `number`.

```c
gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
gpio_set_irq_enabled(BUTTON_B, GPIO_IRQ_EDGE_FALL, true);
```

### **💡 Debouncing**
- Cada botão tem um tempo de **50ms a 100ms** antes de ser aceito novamente.
- Isso impede **leituras múltiplas** causadas por bouncing elétrico.

```c
if (absolute_time_diff_us(*last_time, now) >= 100000) {  // 100ms
    *last_time = now;
    return (gpio_get(gpio) == 0);
}
```

### **💡 Atualização da Matriz WS2812**
- A matriz **não** está na ordem tradicional (0,1,2,...).
- Em vez disso, usamos um **mapeamento real** da disposição física:

```c
static const uint8_t LEDmap[5][5] = {
    {24, 23, 22, 21, 20},
    {15, 16, 17, 18, 19},
    {14, 13, 12, 11, 10},
    { 5,  6,  7,  8,  9},
    { 4,  3,  2,  1,  0}
};
```

---

## 🚀 **Possíveis Melhorias**
✅ Suporte para **mais efeitos visuais** (transições suaves entre números).  
✅ Suporte para **diferentes cores nos números**.  
✅ Modo **"contador automático"** para exibir os números sem precisar dos botões.  
✅ Integração com **sensor externo** (ex.: encoder rotativo ou joystick).  

---

## 📜 **Licença**
Este projeto é de código aberto e pode ser usado livremente para fins educacionais ou pessoais.  
Sinta-se à vontade para contribuir ou modificar conforme necessário!

---

Se você tiver dúvidas ou sugestões, **abra uma issue ou envie um pull request!** 🚀  
Autor: **@kenshindias**

