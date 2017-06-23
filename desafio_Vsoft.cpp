#include <list>
#include <fstream>
#include <math.h>
#include <stdlib.h>
#include <vector>
#include <time.h>
#include <thread>
#include <string>
#include <sstream>
#include <iostream>
#include <stdio.h>

#define NUM_THREADS 2  //Numero de threads que sera utilizado, varia de computador para computador

using namespace std;

float score = 0;
pthread_mutex_t mutex, print;
pthread_cond_t cond;
int maior_quant = 0;

class Ponto{
public:
    float x, y;
    int tipo;
    float angulo;
    bool operator==(Ponto c){// Ponto a == Ponto b
        if(this->x == c.x && this->y == c.y) return true;
        else return false;
    }
};

class Amostra{
    public:
    list<Ponto> listaPontos;
    float xh, xl, yh, yl;
    int numeroPontos;
};

Amostra* salvarAmostras(char* argv){


    cout << "*: " << argv << endl;
    //char nome[] = argv;
    Amostra *am = new Amostra();
    float auxArquivo = 0.0f;
    ifstream file(argv);

    //salva o primeiro elemento do arquivo (Numero de pontos)
    file >> auxArquivo;
    am->numeroPontos = auxArquivo;

    int i = 0;
    Ponto *novoPonto = new Ponto();

    //Salva os pontos na amostra
    while(file >> auxArquivo){
        if(i == 0){
            i++;
            novoPonto->x = auxArquivo;
        }else if(i == 1 ){
            i++;
            novoPonto->y = auxArquivo;
        }else if(i == 2){
            i++;
            novoPonto->angulo = auxArquivo;
        }else if(i == 3 ){
            i = 0;
            novoPonto->tipo = (int)auxArquivo;
            am->listaPontos.push_back(*novoPonto);
        }
    }
}

//Gira a amostra de acordo com o angulo que a respectiva thread possui.
void rotation(Amostra *am, float angulo){

    float c = cos(0.0174533*angulo);
    float s = sin(0.0174533*angulo);

    list<Ponto>::iterator it = am->listaPontos.begin();
    while(it != am->listaPontos.end()){
        float x = it->x, y = it->y;
        it->x = x*c - y*s;///xcos(1А em rad) - ysen(1А em rad);
        it->y = x*s + y*c;///xsen(1А em rad) + ycos(1А em rad);

        it->angulo += 0.0174533*angulo;

        if(it->angulo >= 6.2831850) it->angulo -= 6.2831850; ///se maior que 360А
        it++;
    }

}

///Se os pontos forem iguais, receberam 100 pontos
float calcular_score(Ponto p2, Ponto p1, float angulo, bool tipo){

    float distanciaX, distanciaY, pontuacao = 0.0f;

    distanciaX = fabs(p2.x - p1.x);
    distanciaY = fabs(p2.y - p1.y);

    float aux;
    double graus_maximo = 0.0174533*4.5;
    pontuacao += 25 - (25*distanciaX/9);
    pontuacao += 25 - (25*distanciaY/9);
    pontuacao += 25 - ((25*angulo)/graus_maximo);
    pontuacao += (tipo) ? 25 : 10;

    return (pontuacao);
}

float comparation(Amostra *am1, Amostra *am2){

    int maior_n_pontos = (am1->numeroPontos >= am2->numeroPontos) ? am1->numeroPontos : am2->numeroPontos;

    float score_temp = 0.0f; ///score temporario
    list<Ponto> copAm1 = am1->listaPontos; ///lista dos pontos da amostra 1
    float esquerda, direita;
    Ponto *maisProximo;
    float angulo;
    float pontuacao_temp = 0; ///score por cada comparaçao entre os pontos
    float aux;
    Ponto teste;
    bool existe = false;

    list<Ponto> lista_pontos_usados;
    list<Ponto>::iterator lpu;

    list<Ponto>::iterator it = am2->listaPontos.begin();
    list<Ponto>::iterator it1;
    while(it != am2->listaPontos.end()){

        maisProximo = new Ponto();
        maisProximo->tipo = 3;
        pontuacao_temp = 0;
        existe = false;

        it1 = copAm1.begin();
        while(it1 != copAm1.end()){

            esquerda = fabs(it1->angulo - it->angulo);
            direita = fabs(it1->angulo - (6.28319 + it->angulo));

            aux = calcular_score(*it1,*it,
                                 ((esquerda <= direita) ? esquerda : direita),
                                 ((it->tipo == it1->tipo) ? true : false));

            if( aux > pontuacao_temp ){
                pontuacao_temp = aux;
                teste = *it1;
            }
            it1++;
        }

        lpu = lista_pontos_usados.begin();
        while(lpu != lista_pontos_usados.end()){
            if(*lpu == teste ){
                existe = true;
                lpu = lista_pontos_usados.end();
            }else lpu++;
        }

        if(!existe){
            *maisProximo = teste;
            lista_pontos_usados.push_back(teste);
        }

        if(pontuacao_temp > 0){
            score_temp += pontuacao_temp;
        }
        if(maisProximo->tipo = 3) score_temp += 5; /// se nao encontrar nada, recebe uma pontuacao minima
        it++;
        free(maisProximo);
    }

    double res = score_temp / maior_n_pontos;

    return res;
}

void distanciaComMaiorPontuacao(float res[][2], Amostra *am1, Amostra *am2){

    list<Ponto> cop_am2_pontos = am2->listaPontos;
    list<Ponto> cop_am1_pontos = am1->listaPontos;
    list<Ponto>::iterator it2 = cop_am2_pontos.begin();
    list<Ponto>::iterator it1 = cop_am1_pontos.begin();

    float totalDistancia[am2->numeroPontos][2];

    double menorAngulo, aux;
    double esquerda = fabs(it1->angulo - it2->angulo);
    double direita = fabs(it1->angulo - (6.28319 + it2->angulo));
    double score_aux;
    menorAngulo = (esquerda < direita) ? esquerda : direita;
    totalDistancia[0][0] = it1->x - it2->x;
    totalDistancia[0][1] = it1->y - it2->y;
    score_aux = calcular_score(*it1,*it2,menorAngulo,((it1->tipo == it2->tipo) ? true : false));
    it1++;
    int linhaAtual = 0;
    while(it2 != cop_am2_pontos.end()){

        while(it1 != cop_am1_pontos.end()){

            esquerda = fabs(it1->angulo - it2->angulo);
            direita = fabs(it1->angulo - (6.28319 + it2->angulo));

            aux = (esquerda < direita) ? esquerda : direita;

            if( ( aux <= menorAngulo ) && ( score_aux < calcular_score(*it1,*it2,menorAngulo,((it1->tipo == it2->tipo) ? true : false)) ) ) {
                menorAngulo = aux;
                score_aux = calcular_score(*it1,*it2,menorAngulo,((it1->tipo == it2->tipo) ? true : false));
                totalDistancia[linhaAtual][0] = it1->x - it2->x;
                totalDistancia[linhaAtual][1] = it1->y - it2->y;
            }
            it1++;
        }

        it1 = cop_am1_pontos.begin();
        it2++;

        esquerda = fabs(it1->angulo - it2->angulo);
        direita = fabs(it1->angulo - (6.28319 + it2->angulo));

        menorAngulo = (esquerda < direita) ? esquerda : direita;
        score_aux = calcular_score(*it1,*it2,menorAngulo,((it1->tipo == it2->tipo) ? true : false));

        linhaAtual++;
    }
    for(int i = 0; i < am2->numeroPontos; i++){
        res[i][0] = totalDistancia[i][0];
        res[i][1] = totalDistancia[i][1];
    }
}

void deslocar(Amostra *am, float x, float y){

    list<Ponto>::iterator it = am->listaPontos.begin();
    while(it != am->listaPontos.end()){

        it->x += x;
        it->y += y;

        it++;
    }
    am->xh += x;
    am->xl += x;
    am->yh += y;
    am->yl += y;
}

void function_thread(Amostra *am1, Amostra *am2, float angulo){

    Amostra comparacao = *am2;

    rotation(&comparacao,angulo);

    float melhorDistancia[comparacao.numeroPontos][2];

    distanciaComMaiorPontuacao(melhorDistancia, am1, &comparacao);

    float aux, score_local = 0;

    for(int i = 0; i < am2->numeroPontos; i++){
        comparacao = *am2;
        rotation(&comparacao,angulo);
        deslocar(&comparacao,melhorDistancia[i][0],melhorDistancia[i][1]);
        aux = comparation(am1,&comparacao);
        if(aux > score_local) score_local = aux;
    }

    pthread_mutex_lock(&mutex);

    if(score < score_local) score = score_local;

    pthread_mutex_unlock(&mutex);
}

//Função inicial do programa. Essa função faz o controle das threads
void control(Amostra *am1, Amostra *am2){

    thread t[360+NUM_THREADS]; // como são 360 graus, fizemos uma thread para cada angulo.
    int i = 0, rodada = 1;

    while( i < 360 ){
        for(int k = i; k < i+NUM_THREADS; k++){

            t[k] = thread(function_thread,am1,am2,(k)); //chama a função function_thread
        }

        for(int k = i; k < i+NUM_THREADS; k++){
            t[k].join();
        }
        i = i+NUM_THREADS;
        rodada++;

        if(score == 100) break;
    }

}

int main(int argc, char** argv){

    //As amostras 1 e 2 sao lidas dos arquivos através da função salvarAmostras()
    Amostra *am1 = salvarAmostras(argv[1]);
    Amostra *am2 = salvarAmostras(argv[2]);

    control(am1,am2); //Função inicial do algoritmo.

    FILE *file = fopen(argv[3],"a");

    if(file == nullptr) {

        cout << "erro ao abrir o arquivo" << endl;
        return 0;
    }else{

        ostringstream conversor;
        string resposta_texto = argv[1];
        resposta_texto += ";";
        resposta_texto += argv[2];
        resposta_texto += (score != 0) ? ";OK;" : ";FAIL;";

        conversor << (int)score;
        resposta_texto += conversor.str();
        resposta_texto += "\n";

        fwrite(resposta_texto.c_str(),resposta_texto.size(),1,file);
        fclose(file);
    }
    return 0;
}
