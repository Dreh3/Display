#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h" //Deboucing e interrupções

//Para o display
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "font.h"
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
ssd1306_t ssd; // Inicializa a estrutura do display

//Arquivo .pio
#include "pio_matrix.pio.h"
#define IS_RGBW false
#define MatrizLeds 7

PIO pio = pio0;
int sm =0;
static volatile uint32_t tempo_anterior = 0; //Deboucing

//Definindo pinos
#define ButtonA 5       //Pino do botão A
#define ButtonB 6       //Pino do botão B
#define LedG 11         //Pino do Led Verde
#define LedB 12         //Pino do Led Azul
#define LedR 13         //Pino do Led Vermelho

//Função para iniciar pinos e configurá-los como entrada/saída
void config_pinos(){
    //Inicialização
    gpio_init(ButtonA);
    gpio_init(ButtonB);
    gpio_init(LedR);
    gpio_init(LedG);
    gpio_init(LedB);

    //Entrada/Saída
    gpio_set_dir(ButtonA, GPIO_IN); //Definindo Botões como entrada
    gpio_set_dir(ButtonB, GPIO_IN);
    gpio_set_dir(LedR, GPIO_OUT);   //Definindo Leds como saída
    gpio_set_dir(LedG, GPIO_OUT);
    gpio_set_dir(LedB, GPIO_OUT);

    gpio_pull_up(ButtonA);  //Definindo Botões como Pull-Up
    gpio_pull_up(ButtonB);
    
    //Leds RGB inicialmente apagados
    gpio_put(LedG,0);
    gpio_put(LedB,0);
    gpio_put(LedR,0);
};

                                //Configurações para a matriz de Leds

//Definindo struct para cores personalizadas
typedef struct {
    double red;
    double green;
    double blue;
}Led_RGB;

//Definindo tipo Cor
typedef Led_RGB COR_RGB;

// Definição de tipo da matriz de leds
typedef Led_RGB Matriz_leds[5][5];

//Retorno o valor binário para a cor passada por parâmetro
uint32_t cor_binario (double b, double r, double g)
{
  unsigned char R, G, B;
  R = r * 255; 
  G = g * 255;  
  B = b * 255;
  return (G << 24) | (R << 16) | (B << 8);
};

//Função responsável por acender os leds desejados 
void acender_leds(Matriz_leds matriz){
    //Primeiro for para percorrer cada linha
    for (int linha =4;linha>=0;linha--){
        /*
        Devido à ordem de disposição dos leds na matriz de leds 5X5, é necessário
        ter o cuidado para imprimir o desenho na orientação correta. Assim, o if abaixo permite o 
        desenho saia extamente como projetado.
        */

        if(linha%2){                             //Se verdadeiro, a numeração das colunas começa em 4 e decrementam
            for(int coluna=0;coluna<5;coluna++){
                uint32_t cor = cor_binario(matriz[linha][coluna].blue,matriz[linha][coluna].red,matriz[linha][coluna].green);
                pio_sm_put_blocking(pio, sm, cor);
            };
        }else{                                      //Se falso, a numeração das colunas começa em 0 e incrementam
            for(int coluna=4;coluna>=0;coluna--){
                uint32_t cor = cor_binario(matriz[linha][coluna].blue,matriz[linha][coluna].red,matriz[linha][coluna].green);
                pio_sm_put_blocking(pio, sm, cor);
            };
        };
    };
};

//Função com os frames dos números



// Prototipo da função de interrupção
static void interrupcao_Botao(uint gpio, uint32_t events);

int main()
{
    stdio_init_all();

    config_pinos();

    //Configurações para matriz de leds
    uint offset = pio_add_program(pio, &pio_matrix_program);
    pio_matrix_program_init(pio, sm, offset, MatrizLeds, 800000, IS_RGBW);

      // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA); // Pull up the data line
    gpio_pull_up(I2C_SCL); // Pull up the clock line
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display
    
    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Configuração da interrupção com callback
    gpio_set_irq_enabled_with_callback(ButtonA, GPIO_IRQ_EDGE_FALL, true, &interrupcao_Botao);
    gpio_set_irq_enabled_with_callback(ButtonB, GPIO_IRQ_EDGE_FALL, true, &interrupcao_Botao);

    while (true) {
        
    }
}

void interrupcao_Botao (uint gpio, uint32_t events)
{
    // Obtém o tempo atual em microssegundos
    uint32_t tempo_atual = to_us_since_boot(get_absolute_time());
    bool state_ledVerde = gpio_get(LedG);
    bool state_ledAzul = gpio_get(LedB);

    if (tempo_atual - tempo_anterior > 300000) // 200 ms de debouncing
    {
        tempo_anterior = tempo_atual; // Atualiza o tempo do último evento
        bool cor = true;
        if(gpio==ButtonA){              //Verifica qual botão foi pressionado
            printf("Botão A pressionado!\n");       //Nesse caso botão A 
            state_ledVerde = gpio_get(LedG);
            printf("%d", state_ledVerde);
            printf("Invertendo sinal do led verde...\n");
            gpio_put(LedG, !state_ledVerde);          //Alterna estado do led verde
            //Para o display
            cor = !cor;
            // Atualiza o conteúdo do display com animações
            ssd1306_fill(&ssd, !cor); // Limpa o display
            if(state_ledVerde){
                //Mostrar mensagem LED VERDE DESLIGADO
                printf("Led Verde desligado\n");
                ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
                ssd1306_draw_string(&ssd, "LED VERDE", 8, 10); // Desenha uma string
                ssd1306_draw_string(&ssd, "DESLIGADO", 30, 30); // Desenha uma string      
                ssd1306_send_data(&ssd); // Atualiza o display
            }else{
                //Mostrar mensagem LED VERDE LIGADO
                printf("Led Verde ligado\n");
                ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
                ssd1306_draw_string(&ssd, "LED VERDE", 8, 10); // Desenha uma string
                ssd1306_draw_string(&ssd, "  LIGADO ", 30, 30); // Desenha uma string      
                ssd1306_send_data(&ssd); // Atualiza o display
            }

        }else{
            //Para o display
            cor = !cor;
            // Atualiza o conteúdo do display com animações
            ssd1306_fill(&ssd, !cor); // Limpa o display
            printf("Botão B pressionado!\n");       //Nesse caso botão B
            state_ledAzul = gpio_get(LedB);         //Salva estado atual do led
            printf("Invertendo sinal do led azul...\n");
            gpio_put(LedB, !state_ledAzul);          //Alterna estado do led azul
            //Para o display
            if(state_ledAzul){
                //Mostrar mensagem LED AZUL DESLIGADO
                printf("Led Azul desligado\n");
                ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
                ssd1306_draw_string(&ssd, "LED AZUL", 8, 10); // Desenha uma string
                ssd1306_draw_string(&ssd, "DESLIGADO", 30, 30); // Desenha uma string      
                ssd1306_send_data(&ssd); // Atualiza o display
            }else{
                //Mostrar mensagem LED AZUL LIGADO
                printf("Led Azul ligado\n");
                 ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
                ssd1306_draw_string(&ssd, "LED AZUL", 8, 10); // Desenha uma string
                ssd1306_draw_string(&ssd, "  LIGADO ", 30, 30); // Desenha uma string      
                ssd1306_send_data(&ssd); // Atualiza o display
            }
        }
    }
}
