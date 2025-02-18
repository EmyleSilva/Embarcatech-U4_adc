# Uso de Joystick com ADC no RP2040

Projeto criado com o objetivo de consolidar os conceitos sobre o uso de conversores analógico-digitais (ADC) no RP2040, além de explorar as funcionalidades da placa de desenvolvimento BitDogLab.

## Objetivos
- Compreender o funcionamento do conversor analógico-digital (ADC) no RP2040.
- Utilizar o PWM para controlar a intensidade de dois LEDs RGB com base nos valores do joystick.
- Representar a posição do joystick no display SSD1306 por meio de um quadrado móvel.
- Aplicar o protocolo de comunicação I2C na integração com o display.

## Descrição do Projeto
Para o projeto, foram utilizados os seguintes componentes conectados à placa BitDogLab:
- **LED RGB**, com os pinos conectados às GPIOs (11, 12 e 13).
- **Botão do Joystick** conectado à GPIO 22.
- **Joystick** conectado às GPIOs 26 e 27.
- **Botão A** conectado à GPIO 5.
- **Botão B** conectado à GPIO 6.
- **Display SSD1306** conectado via I2C (GPIO 14 e GPIO 15).

## Funcionalidades do Projeto
1. **Entradas analógicas correspondentes aos eixos X e Y do Joystick**:
   - A intensidade luminosa dos LEDs RGB é controlada de acordo com os valores de entrada do joystick.
   - **LED AZUL**: Tem seu brilho ajustado conforme o valor do eixo Y. Quando o joystick estiver solto, o LED permanece apagado. À medida que o joystick é movimentado no eixo Y, o LED aumenta seu brilho gradualmente, atingindo a intensidade máxima nos extremos (0 e 4095).
   - **LED VERMELHO**: Segue o mesmo princípio do LED AZUL, mas de acordo com o eixo X.
   - A variação da intensidade luminosa dos LEDs é controlada via PWM, para garantir uma variação suave.

2. **Exibição no Display SSD1306**:
   - Um quadrado de 8x8 pixels é exibido no display, inicialmente centralizado, movendo-se de forma proporcional aos valores capturados pelo joystick.

3. **Acionamento de Botões**:
   - **Botão do Joystick**: Alterna o estado do LED Verde a cada acionamento e modifica a borda exibida no display para indicar que foi pressionado.
   - **Botão A**: Ativa ou desativa os LEDs PWM a cada acionamento.
   - **Botão B**: Coloca a placa no modo de Bootsel.

## Requisitos do Projeto
- [X] Uso de interrupções para todas as funcionalidades relacionadas aos botões.
- [X] Implementação do tratamento de bouncing dos botões via software.
- [X] Utilização do Display SSD1306 e do protocolo I2C para comunicação com o display.
- [X] Organização do código.

## Vídeo de Demonstração
O vídeo demonstrativo do projeto está disponível no YouTube e pode ser acessado através do link abaixo:  
[Link para o vídeo](https://youtu.be/VVpNzHf7Fh0?si=VUB-1-4bW-lpWyld)

## Como Compilar e Executar
1. **Pré-requisitos**:
   - Ter o ambiente de desenvolvimento para o Raspberry Pi Pico configurado (compilador, SDK, etc.).
   - CMake instalado.

2. **Compilação**:
   - Clone o repositório ou baixe os arquivos do projeto.
   - Navegue até a pasta do projeto e crie uma pasta de build:
     ```bash
     mkdir build
     cd build
     ```
   - Execute o CMake para configurar o projeto:
     ```bash
     cmake ..
     ```
   - Compile o projeto:
     ```bash
     make
     ```

3. **Upload para a placa**:
   - Conecte o Raspberry Pi Pico ao computador.
   - Copie o arquivo `.uf2` gerado para a placa.

## Simulação no Wokwi
Para visualizar a simulação do projeto no Wokwi:
1. Instale e configure o simulador Wokwi seguindo as instruções encontradas no link a seguir:  
   [Introdução ao Wokwi para VS Code](https://docs.wokwi.com/pt-BR/vscode/getting-started).
2. Abra o arquivo `diagram.json` no VS Code.
3. Clique em "Start Simulation".