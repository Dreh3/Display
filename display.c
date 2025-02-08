#include <stdio.h>
#include <string.h>
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
void numeros(char numero){

    //Declarando variáveis necessárias para modificar intensidade
    int inten =1, inten2 =0.5;

    //Definindo cores e tonalidades
    COR_RGB apagado = {0.0,0.0,0.0};
    COR_RGB vermelhoForte = {0.5*inten,0.0,0.0};
    COR_RGB vermelhoClaro = {0.2*inten2,0.0,0.0};
    COR_RGB azulForte = {0.0,0.0,0.5*inten};
    COR_RGB azulClaro = {0.0,0.0,0.2*inten2};
    COR_RGB verdeForte = {0.0,0.5*inten,0.0};
    COR_RGB verdeClaro = {0.0,0.2*inten2,0.0};
    COR_RGB marromForte = {0.345*inten2,0.065*inten2,0.065*inten2};
    COR_RGB marromClaro = {0.69*inten,0.13*inten,0.13*inten};
    COR_RGB magentaForte = {0.5*inten,0.0,0.5*inten};
    COR_RGB magentaClaro = {0.25*inten2,0.0,0.25*inten2};


    //Foram estabelecidos 6 frames para cada um dos números

    //Matriz que mostrará o zero com transição de tonalidade da esquerda para a direita
    Matriz_leds zero =
        {{apagado, vermelhoClaro, vermelhoClaro, vermelhoClaro, apagado},
        {vermelhoClaro, apagado, apagado,apagado, vermelhoClaro},
        {vermelhoClaro, apagado, vermelhoClaro,apagado, vermelhoClaro},
        {vermelhoClaro, apagado, apagado,apagado, vermelhoClaro},
        {apagado, vermelhoClaro, vermelhoClaro, vermelhoClaro,apagado}};
    //Matriz que mostrará o um com transição de tonalidade da esquerda para a direita
    Matriz_leds um =
        {{apagado, apagado, azulClaro, apagado, apagado},
        {apagado, azulClaro, azulClaro,apagado, apagado},
        {apagado, apagado, azulClaro,apagado, apagado},
        {apagado, apagado, azulClaro,apagado, apagado},
        {azulClaro, azulClaro, azulClaro, azulClaro,azulClaro}};
    //Matriz que mostrará o dois com transição de tonalidade da direita para a esquerda
    Matriz_leds dois =
        {{apagado, magentaForte, magentaForte, magentaForte, apagado},
        {magentaForte, apagado, apagado,apagado, magentaForte},
        {apagado, apagado, magentaForte,magentaForte, apagado},
        {apagado, magentaForte, apagado,apagado, apagado},
        {magentaForte, magentaForte, magentaForte, magentaForte,magentaForte}};
    //Matriz que mostrará o três com transição de tonalidade da direita para a esquerda
    Matriz_leds tres =
        {{apagado, azulForte, azulForte, azulForte, apagado},
        {azulForte, apagado, apagado,apagado, azulForte},
        {apagado, apagado, azulForte,azulForte, apagado},
        {azulForte, apagado, apagado,apagado, azulForte},
        {apagado, azulForte, azulForte, azulForte,apagado}};
    //Matriz que mostrará o quatro com transição de tonalidade de cima para baixo
    Matriz_leds quatro =
        {{apagado, apagado, apagado, marromClaro, apagado},
        {apagado, apagado, marromClaro,marromClaro, apagado},
        {apagado, marromClaro, apagado,marromClaro, apagado},
        {marromClaro, marromClaro, marromClaro,marromClaro, marromClaro},
        {apagado, apagado, apagado, marromClaro, apagado}};
    //Matriz que mostrará o cinco com transição de tonalidade de baixo para cima
    Matriz_leds cinco =
        {{azulClaro, azulClaro, azulClaro, azulClaro, apagado},
        {azulClaro, apagado, apagado,apagado, apagado},
        {azulClaro, azulClaro, azulClaro,azulClaro, apagado},
        {apagado, apagado, apagado,apagado, azulClaro},
        {azulClaro, azulClaro, azulClaro, azulClaro, apagado}};
    //Matriz que mostrará o seis com transição de tonalidade de cima para baixo
    Matriz_leds seis =
        {{apagado, magentaClaro, magentaClaro, magentaClaro, apagado},
        {magentaClaro, apagado, apagado,apagado, apagado},
        {magentaClaro, magentaClaro, magentaClaro,magentaClaro, apagado},
        {magentaClaro, apagado, apagado,apagado, magentaClaro},
        {apagado, magentaClaro, magentaClaro, magentaClaro,apagado}};
    //Matriz que mostrará o sete com transição de tonalidade de cima para baixo
    Matriz_leds sete =
        {{verdeForte, verdeForte, verdeForte, verdeForte, verdeForte},
        {apagado, apagado, apagado,apagado, verdeForte},
        {apagado, apagado, apagado,verdeForte, apagado},
        {apagado, apagado, verdeForte,apagado, apagado},
        {apagado, apagado, verdeForte, apagado,apagado}};
    //Matriz que mostrará o oito com transição de tonalidade de baixo para cima
    Matriz_leds oito =
        {{apagado, magentaForte, magentaForte, magentaForte, apagado},
        {magentaForte, apagado, apagado,apagado, magentaForte},
        {apagado, magentaForte, magentaForte,magentaForte, apagado},
        {magentaForte, apagado, apagado,apagado, magentaForte},
        {apagado, magentaForte, magentaForte, magentaForte, apagado}};
    //Matriz que mostrará o nove com transição de tonalidade de cima para baixo
    Matriz_leds nove =
        {{apagado, marromForte, marromForte, marromForte, apagado},
        {marromForte, apagado, apagado,apagado, marromForte},
        {apagado, marromForte, marromForte,marromForte, marromForte},
        {apagado, apagado, apagado,apagado, marromForte},
        {apagado, marromForte, marromForte, marromForte, apagado}};

    Matriz_leds limpar = 
        {{apagado,apagado,apagado,apagado,apagado},
        {apagado,apagado,apagado,apagado,apagado},
        {apagado,apagado,apagado,apagado,apagado},
        {apagado,apagado,apagado,apagado,apagado},
        {apagado,apagado,apagado,apagado,apagado}};
    //Laço de repetição para mostrar todos os frames na matriz de Leds
        switch(numero){
            case '0':
                    acender_leds(zero);
                break;
            case '1':
                    acender_leds(um);
                break;
            case '2':
                    acender_leds(dois);
                break;
            case '3':
                    acender_leds(tres);
                break;
            case '4':
                    acender_leds(quatro);
                break;
            case '5':
                    acender_leds(cinco);
                    break;
            case '6':
                    acender_leds(seis);
                    break;
            case '7':
                    acender_leds(sete);
                    break;
            case '8':
                    acender_leds(oito);
                    break;
            case '9': 
                    acender_leds(nove);
                    break;
            default:
                acender_leds(limpar);
                break;
            }; 

};

//Função para mostrar no display 
void limpar_display(){
    bool cor = true;
    cor = !cor;
    // Atualiza o conteúdo do display com animações
    ssd1306_fill(&ssd, !cor); // Limpa o display
}

void mensagem_display(char mensagem[], char mensagem2[], char c){
    bool cor = true;
    cor = !cor;
    ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
    ssd1306_draw_string(&ssd, mensagem, 8, 10); // Desenha uma string 
    if(c=='\0'){
        ssd1306_draw_string(&ssd, mensagem2, 30, 30); // Desenha uma string
    }else{
        ssd1306_draw_char(&ssd, c, 35, 35);
    } 
    ssd1306_send_data(&ssd); // Atualiza o display
};



// Prototipo da função de interrupção
static void interrupcao_Botao(uint gpio, uint32_t events);

int main()
{
    stdio_init_all();

    config_pinos();

    char mensagem = '\0';
    char frase[15];
    char frase2[15];
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
        printf("\nDigite um caractere:");
        scanf("%c",&mensagem);
        printf("\nVocê digitou: %c\n", mensagem);
        numeros(mensagem);
        limpar_display();
        strcpy(frase,"LETRA DIGITADA");
        mensagem_display(frase, frase2, mensagem);
        sleep_ms(1000);
    };
};

void interrupcao_Botao (uint gpio, uint32_t events)
{
    // Obtém o tempo atual em microssegundos
    uint32_t tempo_atual = to_us_since_boot(get_absolute_time());
    bool state_ledVerde = gpio_get(LedG);
    bool state_ledAzul = gpio_get(LedB);

    if (tempo_atual - tempo_anterior > 300000) // 200 ms de debouncing
    {
        tempo_anterior = tempo_atual; // Atualiza o tempo do último evento
        char mensagem = '\0';
        char frase[15];
        char frase2[15];

        if(gpio==ButtonA){              //Verifica qual botão foi pressionado
            printf("\n\nBotão A foi pressionado!\n");       //Nesse caso botão A 
            printf("Invertendo sinal do led verde...\n");
            state_ledVerde = gpio_get(LedG);
            gpio_put(LedG, !state_ledVerde);          //Alterna estado do led verde
            limpar_display();
            if(state_ledVerde){
                //Mostrar mensagem LED VERDE DESLIGADO
                printf("Led Verde desligado\n\n");
                strcpy(frase,"LED VERDE");
                strcpy(frase2,"DESLIGADO");
                mensagem_display(frase, frase2, mensagem);
            }else{
                //Mostrar mensagem LED VERDE LIGADO
                printf("Led Verde ligado\n\n");
                strcpy(frase,"LED VERDE");
                strcpy(frase2,"LIGADO");
                mensagem_display(frase, frase2, mensagem);
            };

        }else{
            printf("\n\nBotão B foi pressionado!\n");       //Nesse caso botão B
            printf("Invertendo sinal do led azul...\n");
            state_ledAzul = gpio_get(LedB);         //Salva estado atual do led
            gpio_put(LedB, !state_ledAzul);          //Alterna estado do led azul
            limpar_display();
            if(state_ledAzul){
                //Mostrar mensagem LED AZUL DESLIGADO
                printf("Led Azul desligado\n\n");
                strcpy(frase,"LED AZUL");
                strcpy(frase2,"DESLIGADO");
                mensagem_display(frase, frase2, mensagem);
            }else{
                //Mostrar mensagem LED AZUL LIGADO
                printf("Led Azul ligado\n\n");
                strcpy(frase,"LED AZUL");
                strcpy(frase2,"LIGADO");
                mensagem_display(frase, frase2, mensagem);
            };
        };
        printf("\nDigite um caractere:");
    };
};
