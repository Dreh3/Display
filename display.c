//Código desenvolvido por Andressa Sousa Fonseca

/*
O presente código utiliza interrupções para executar funções para os botões:
    O botão A ao ser pressionado alterna o estado do led verde.
    O botão B ao ser pressionado alterna o estado do led azul.
Já o código principal tem as seguintes funcionalidades:
    1 - É possível entrar com uma String de até 8 caracteres pelo monitor serial,
        esse string será exibida no display.
    2 - Caso seja digitado apenas um número, isoladamente, ele será exibido na 
        matriz de leds 5x5.
Ademais, a ações de mudança dos leds também sinalizadas por mensagens no display.
E ao digitar nova mensagem o Led Vermelho pisca.
*/

#include <stdio.h>
#include <string.h> //Para manipular strings
#include "pico/stdlib.h"

#include "hardware/timer.h" //Deboucing e interrupções

//Para o display
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "font.h"
#define I2C_PORT i2c1
#define I2C_SDA 14 //pinos para comunicação I2C
#define I2C_SCL 15 //pinos para comunicação I2C
#define endereco 0x3C
ssd1306_t ssd; // Inicializa a estrutura do display

//Arquivo .pio
#include "pio_matrix.pio.h"
#define IS_RGBW false 
#define MatrizLeds 7 //Pino para matriz de leds

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
void numeros(char numero[]){

    //Declarando variáveis necessárias para modificar intensidade
    int inten =4, inten2 =2;

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


    //Foi estabelecido 1 frame para cada um dos números

    //Matriz que mostrará o zero
    Matriz_leds zero =
        {{apagado, vermelhoClaro, vermelhoClaro, vermelhoClaro, apagado},{vermelhoClaro, apagado, apagado,apagado, vermelhoClaro},{vermelhoClaro, apagado, vermelhoClaro,apagado, vermelhoClaro},{vermelhoClaro, apagado, apagado,apagado, vermelhoClaro},{apagado, vermelhoClaro, vermelhoClaro, vermelhoClaro,apagado}};
    //Matriz que mostrará o um 
    Matriz_leds um =
        {{apagado, apagado, azulClaro, apagado, apagado},{apagado, azulClaro, azulClaro,apagado, apagado},{apagado, apagado, azulClaro,apagado, apagado},{apagado, apagado, azulClaro,apagado, apagado},{azulClaro, azulClaro, azulClaro, azulClaro,azulClaro}};
    //Matriz que mostrará o dois 
    Matriz_leds dois =
        {{apagado, magentaForte, magentaForte, magentaForte, apagado},{magentaForte, apagado, apagado,apagado, magentaForte},{apagado, apagado, magentaForte,magentaForte, apagado},{apagado, magentaForte, apagado,apagado, apagado},{magentaForte, magentaForte, magentaForte, magentaForte,magentaForte}};
    //Matriz que mostrará o três 
    Matriz_leds tres =
        {{apagado, azulForte, azulForte, azulForte, apagado},{azulForte, apagado, apagado,apagado, azulForte},{apagado, apagado, azulForte,azulForte, apagado},{azulForte, apagado, apagado,apagado, azulForte},{apagado, azulForte, azulForte, azulForte,apagado}};
    //Matriz que mostrará o quatro 
    Matriz_leds quatro =
        {{apagado, apagado, apagado, marromClaro, apagado},{apagado, apagado, marromClaro,marromClaro, apagado},{apagado, marromClaro, apagado,marromClaro, apagado},{marromClaro, marromClaro, marromClaro,marromClaro, marromClaro},{apagado, apagado, apagado, marromClaro, apagado}};
    //Matriz que mostrará o cinco 
    Matriz_leds cinco =
        {{azulClaro, azulClaro, azulClaro, azulClaro, apagado},{azulClaro, apagado, apagado,apagado, apagado},{azulClaro, azulClaro, azulClaro,azulClaro, apagado},{apagado, apagado, apagado,apagado, azulClaro},{azulClaro, azulClaro, azulClaro, azulClaro, apagado}};
    //Matriz que mostrará o seis 
    Matriz_leds seis =
        {{apagado, magentaClaro, magentaClaro, magentaClaro, apagado},{magentaClaro, apagado, apagado,apagado, apagado},{magentaClaro, magentaClaro, magentaClaro,magentaClaro, apagado},{magentaClaro, apagado, apagado,apagado, magentaClaro},{apagado, magentaClaro, magentaClaro, magentaClaro,apagado}};
    //Matriz que mostrará o sete 
    Matriz_leds sete =
        {{verdeForte, verdeForte, verdeForte, verdeForte, verdeForte},{apagado, apagado, apagado,apagado, verdeForte},{apagado, apagado, apagado,verdeForte, apagado},{apagado, apagado, verdeForte,apagado, apagado},{apagado, apagado, verdeForte, apagado,apagado}};
    //Matriz que mostrará o oito 
    Matriz_leds oito =
        {{apagado, magentaForte, magentaForte, magentaForte, apagado},{magentaForte, apagado, apagado,apagado, magentaForte},{apagado, magentaForte, magentaForte,magentaForte, apagado},{magentaForte, apagado, apagado,apagado, magentaForte},{apagado, magentaForte, magentaForte, magentaForte, apagado}};
    //Matriz que mostrará o nove 
    Matriz_leds nove =
        {{apagado, marromForte, marromForte, marromForte, apagado},{marromForte, apagado, apagado,apagado, marromForte},{apagado, marromForte, marromForte,marromForte, marromForte},{apagado, apagado, apagado,apagado, marromForte},{apagado, marromForte, marromForte, marromForte, apagado}};

    Matriz_leds limpar = 
        {{apagado,apagado,apagado,apagado,apagado},{apagado,apagado,apagado,apagado,apagado},{apagado,apagado,apagado,apagado,apagado},{apagado,apagado,apagado,apagado,apagado},{apagado,apagado,apagado,apagado,apagado}};

    //Laço de repetição para mostrar todos os frames na matriz de Leds
    if(!strcmp(numero, "0")){
        acender_leds(zero);
    }else if(!strcmp(numero, "1")){
        acender_leds(um);
    }else if(!strcmp(numero, "2")){
        acender_leds(dois);
    }else if(!strcmp(numero, "3")){
        acender_leds(tres);
    }else if(!strcmp(numero, "4")){
        acender_leds(quatro);
    }else if(!strcmp(numero, "5")){
        acender_leds(cinco);
    }else if(!strcmp(numero, "6")){
        acender_leds(seis);
    }else if(!strcmp(numero, "7")){
        acender_leds(sete);
    }else if(!strcmp(numero, "8")){
        acender_leds(oito);
    }else if(!strcmp(numero, "9")){
        acender_leds(nove);
    }else {
        acender_leds(limpar);
    };
};

//Função para limpar display
void limpar_display(){
    bool cor = true; //Controla cor do display para o fundo contrastar com letra
    cor = !cor;
    ssd1306_fill(&ssd, !cor); // Limpa o display
}

//Função para mostrar mensagem no display
void mensagem_display(char mensagem[], char mensagem2[]){
    bool cor = true;    //Controla cor do display para o fundo contrastar com letra
    cor = !cor;
    ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
    ssd1306_draw_string(&ssd, mensagem, 8, 10); // Desenha a primeira string 
    ssd1306_draw_string(&ssd, mensagem2, 30, 30); // Desenha a segunda string
    ssd1306_send_data(&ssd); // Atualiza o display
};

void led_vermelho(){ //Picar led Vermelho ao mostrar nova mensagem
    gpio_put(LedR,1);
    sleep_ms(1000);
    gpio_put(LedR,0);
};

// Prototipo da função de interrupção
static void interrupcao_Botao(uint gpio, uint32_t events);

int main()
{
    stdio_init_all();   //Inicializa comunicação padrão

    config_pinos(); //Função que configura e inicializa os pinos de leds e botões

    int i = 0;      //Contador para percorrer string
    char c;         //Ler caracteres da string
    char frase[15]; //Strings para armazenar mensagem que serão exibidas
    char frase2[20];
    //Configurações para matriz de leds
    uint offset = pio_add_program(pio, &pio_matrix_program);
    pio_matrix_program_init(pio, sm, offset, MatrizLeds, 800000, IS_RGBW);

      // Inicializando comunicação I2C
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Configura pino SDA para I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Configura pino SCL para I2C
    gpio_pull_up(I2C_SDA); // estabelece sda como pull-up
    gpio_pull_up(I2C_SCL); // estabelece sda como pull-up

    // Inicializa o display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); 
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display
    
    // Apresenta uma mensagem inicial no display
    limpar_display();
    strcpy(frase,"Oi");
    strcpy(frase2, "Espero que goste"); 
    mensagem_display(frase, frase2);

    // Configuração da interrupção com callback
    gpio_set_irq_enabled_with_callback(ButtonA, GPIO_IRQ_EDGE_FALL, true, &interrupcao_Botao);
    gpio_set_irq_enabled_with_callback(ButtonB, GPIO_IRQ_EDGE_FALL, true, &interrupcao_Botao);

    while (true) {

        //Aviso sobre end lining
        printf("\nAVISO: HABILITE END LINING (LF)\n");

        //Mensagem vista apenas no menitor serial
        printf("\nDigite uma palavra ou frase de até 8 caracteres: \n OBS: Sem acentuação ou pontuação\n");
        
        memset(frase2, '\0', sizeof(frase2)); //Garante que a string vai estar limpa para próxima leitura
        i = 0;  // Reinicia o índice da string

        while(i!=9){ //While para ler até o ponto de parada
            scanf("%c", &c);      //Ler cada caractere da string
            if (c == '\n') {
                frase2[i] = '\0';  // Finaliza a string com o caractere nulo
                i=9;               //Finaliza while
            } 
            else if (i < sizeof(frase2) - 1) {
                frase2[i] = c;  // Armazena o caractere na string
                i++;            //Incrmenta o índice
            };
        };

        printf("\n\tVocê digitou: %s\n", frase2); //Mostra o que foi digitado no monitor serial
        limpar_display();
        strcpy(frase,"SUA MENSAGEM");           //Mensagem no display que acompanha o que foi digitado
        numeros(frase2);                        //Chama função que verifica se um número foi digitado
        mensagem_display(frase, frase2);        //Chama função que exibe as frases no display
        led_vermelho();                         //Pica led_vermelho para indicar nova mensagem
    };
};

//Função chamada na interrupção do botão
void interrupcao_Botao (uint gpio, uint32_t events)
{
    // Obtém o tempo atual em microssegundos
    uint32_t tempo_atual = to_us_since_boot(get_absolute_time());
    bool state_ledVerde = gpio_get(LedG);   //Variável que armazena o estado do led verde
    bool state_ledAzul = gpio_get(LedB);    //Variável que armazena o estado do led azul

    if (tempo_atual - tempo_anterior > 300000) // 200 ms de debouncing
    {
        tempo_anterior = tempo_atual; // Atualiza o tempo do último evento
        
        char frase[15];             //Strings para mostrar no display
        char frase2[15];

        if(gpio==ButtonA){              //Verifica qual botão foi pressionado
            printf("\n\nBotão A foi pressionado!\n");       //Nesse caso botão A 
            printf("Invertendo sinal do led verde...\n");
            state_ledVerde = gpio_get(LedG);
            gpio_put(LedG, !state_ledVerde);          //Alterna estado do led verde
            limpar_display();
            if(state_ledVerde){
                //Mostrar mensagem LED VERDE DESLIGADO no monitor e no display
                printf("Led Verde desligado\n\n");
                strcpy(frase,"LED VERDE");
                strcpy(frase2,"DESLIGADO");
                mensagem_display(frase, frase2);
            }else{
                //Mostrar mensagem LED VERDE LIGADO no monitor e no display
                printf("Led Verde ligado\n\n"); 
                strcpy(frase,"LED VERDE");
                strcpy(frase2,"LIGADO");
                mensagem_display(frase, frase2);
            };

        }else{
            printf("\n\nBotão B foi pressionado!\n");       //Nesse caso botão B
            printf("Invertendo sinal do led azul...\n");
            state_ledAzul = gpio_get(LedB);         //Salva estado atual do led
            gpio_put(LedB, !state_ledAzul);          //Alterna estado do led azul
            limpar_display();
            if(state_ledAzul){
                //Mostrar mensagem LED AZUL DESLIGADO no monitor e no display
                printf("Led Azul desligado\n\n");
                strcpy(frase,"LED AZUL");
                strcpy(frase2,"DESLIGADO");
                mensagem_display(frase, frase2);
            }else{
                //Mostrar mensagem LED AZUL LIGADO no monitor e no display
                printf("Led Azul ligado\n\n");
                strcpy(frase,"LED AZUL");
                strcpy(frase2,"LIGADO");
                mensagem_display(frase, frase2);
            };
        };
    };
};
