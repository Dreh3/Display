# Display

__Tarefa 1 - Aula Síncrona 03/02__<br>
Repositório criado com o intuito de realizar a Tarefa 1 da aula Síncrona do dia 03 de fevereiro de 2025 sobre Interfaces de Comunicação Serial.

__Responsável pelo desenvolvimento:__
Andressa Sousa Fonseca

## Descrição Da Tarefa 
__Aplicação de Comunicação serial I2C, Display 128x64, Matriz de Leds 5x5 e Leds RGB__  <br>

__Os Componentes necessários para a execução da atividade são:__
1) Matriz 5x5 de LEDs (endereçáveis) WS2812, conectada à GPIO 7.
2) LED RGB, com os pinos conectados às GPIOs (11, 12 e 13).
3) Botão A conectado à GPIO 5.
4) Botão B conectado à GPIO 6.
5) Display SSD1306 conectado via I2C (GPIO 14 e GPIO15).

__As funcionalidade básicas especificadas para a atividade são:__
1) Modificação da Biblioteca font.h
2) Entrada de caracteres via PC.
3) Interação com o Botão A. 
4) Interação com o Botão B.
5) Mensagens no diplay e no Serial Monitor.


__Uma breve explicação do desenvolvimento e resultados obtidos podem ser vistos no vídeo endereçado no seguine link: [Aplicação de Interaces de Comunicação Serial](https://youtu.be/G0G2UHMJdV8?si=dBe-2z5vWocDKS_s).__

## Detalhamento Do Projeto

### 1. Explicação do funcionamento

O projeto permite utilizar os botões A e B para manipular os leds Azul e Verde, respectivamente. Ambos os botões tem a funcionalidade de alternar os estados de seus respectivos leds. Por exemplo, ao iniciar o código pela primeira vez, todos os leds estão desligados, assim, pressioanando o botão A, o led verde é ligado e, se pressionado novamente, o led é desligado. O mesmo acontece com o botão B. Um detalhe a se ressaltar, é que mensagens informativas aparecem no dispplay indicando a mundança de estados dos leds.

Outra funcionalidade consiste em enviar uma frase de até 8 caracteres pelo Serial Monitor que será mostrada no display da placa. É necessário selecionar o Line Ending do Vs Code com a opção LF para permitir a leitura do enter (\n) e assim a leitura dos caracteres será facilitada. Instruções abaixo:

<div align="center">
  <img src="https://github.com/user-attachments/assets/9a5a91d0-33e1-454a-9edb-52a6e12c4934" alt="line ending" width="300"/>
</div>

O led vermelho pisca quando uma nova mensagem é digitada.

A última funcionalidade consiste em mostrar na matriz de leds 5x5 um desenho quando o caractere digitado correpondenter a um número de 0 a 9. Ocorre apenas quando um único caractere com o número é digitado.

A utilização de dois botões permitiu implementar um tratamento de debouncing via software aliado a rotinas de interupção, detalhadas em aulas anteriores. Foi utilizada a seguinte função de interrupção:

static void interrupcao_Botao(uint gpio, uint32_t events);
```
Na main, a função acima é chamada pela função de interrupção com callback a seguir:
```bash
gpio_set_irq_enabled_with_callback(ButtonA, GPIO_IRQ_EDGE_FALL, true, &interrupcao_Botao);
gpio_set_irq_enabled_with_callback(ButtonB, GPIO_IRQ_EDGE_FALL, true, &interrupcao_Botao);
```
É possível perceber que os dois botões chamam a mesma função, interrupcao_Botao(). O diferenciamento entre os botões é feito dentro da função a partir de um if e consideran-se um tempo de 200ms para aceitar que o botão foi pressionado.

### 2. Observações
Se necessário, é possível ajustar as padrões de intensidade das cores na função numeros(), modificando os valores das variáveis inten e inten2, sendo, o tom mais forte e o mais fraco, respectivamente.

Observação: O simulador Integrado Wokwi permite utilizar apenas as funcionalidades do botões, uma vez que não permite leitura de entrada de dados.
