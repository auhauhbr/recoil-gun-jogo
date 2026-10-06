#ifndef RECOIL_TIPOS_H
#define RECOIL_TIPOS_H

#include <box2d/box2d.h>
#include <raylib.h>
#include <stdbool.h>
#include <stdint.h>

#include "config.h"

typedef enum
{
    ESTADO_MENU,
    ESTADO_JOGANDO,
    ESTADO_PAUSADO,
    ESTADO_VITORIA_FASE,
    ESTADO_INTERVALO_SOBREVIVENCIA,
    ESTADO_DERROTA,
    ESTADO_FINAL,
    ESTADO_FIM_SOBREVIVENCIA
} EstadoJogo;

typedef enum
{
    MODO_CAMPANHA,
    MODO_SOBREVIVENCIA
} ModoJogo;

typedef enum
{
    DONO_JOGADOR,
    DONO_INIMIGO
} Dono;

typedef enum
{
    ARMA_PISTOLA,
    ARMA_REVOLVER,
    ARMA_ESPINGARDA,
    TOTAL_ARMAS
} TipoArma;

typedef enum
{
    POWERUP_DUPLO,
    POWERUP_TRIPLO,
    POWERUP_RICOCHETE,
    POWERUP_VELOCIDADE,
    POWERUP_CAVEIRA
} TipoPowerup;

typedef enum
{
    OBJETO_CENARIO,
    OBJETO_ARMA,
    OBJETO_PROJETIL
} TipoObjetoFisico;

typedef struct
{
    TipoObjetoFisico tipo;
    Dono dono;
    int indice;
} MarcadorColisao;

typedef struct
{
    float velocidade;
    float recuo;
    float impulso_angular;
    float cooldown;
    float tempo_vida;
    int ricochetes;
    int quantidade_projeteis;
    float abertura_graus;
    float largura_visual;
    float altura_visual;
    float largura_colisor;
    float altura_colisor;
    float salto_disparo;
    float salto_chao;
    float cano_offset_x;
    float cano_offset_y;
    const char *nome;
} ConfigArma;

typedef struct
{
    bool ativo;
    bool remover;
    Dono dono;
    TipoArma tipo_arma;
    b2BodyId corpo;
    b2ShapeId forma;
    int ricochetes_restantes;
    float tempo_restante;
    MarcadorColisao marcador;
} Projetil;

typedef struct
{
    Dono dono;
    TipoArma tipo;
    b2BodyId corpo;
    b2ShapeId forma;
    int vida;
    int vida_maxima;
    float cooldown_restante;
    float tempo_flash;
    float tempo_ia;
    MarcadorColisao marcador;
} Arma;

typedef struct
{
    Rectangle area;
    b2BodyId corpo;
    MarcadorColisao marcador;
} Obstaculo;

typedef struct
{
    bool ativo;
    Rectangle area;
    TipoPowerup tipo;
    float pulso;
} Powerup;

typedef struct
{
    bool ativo;
    Vector2 posicao;
    Vector2 velocidade;
    float vida;
    float vida_total;
    float tamanho;
    Color cor;
} Particula;

typedef struct
{
    Texture2D armas[TOTAL_ARMAS];
    Texture2D balas[TOTAL_ARMAS];
    Texture2D muzzle;
    Texture2D faisca;
    Texture2D fumaca;
    Texture2D brilho_powerup;

    Sound tiros[TOTAL_ARMAS];
    Sound ricochete;
    Sound powerup;
    Sound impacto;
    Sound clique;
    Music musica;

    bool audio_pronto;
    bool musica_pronta;
} Assets;

typedef struct
{
    const char *nome;
    TipoArma arma_inimigo;
    Vector2 posicao_jogador;
    Vector2 posicao_inimigo;
    float angulo_jogador;
    float angulo_inimigo;
    int vida_jogador;
    int vida_inimigo;
    int quantidade_obstaculos;
    Rectangle obstaculos[MAX_OBSTACULOS];
    int quantidade_powerups;
    TipoPowerup tipos_powerups[MAX_POWERUPS];
    Rectangle areas_powerups[MAX_POWERUPS];
} DefinicaoFase;

typedef struct
{
    b2WorldId mundo;
    bool mundo_criado;

    EstadoJogo estado;
    ModoJogo modo;
    int fase_atual;
    int pontuacao;
    bool sem_audio;

    int onda_sobrevivencia;
    int vida_sobrevivencia;
    int recorde_sobrevivencia;
    int combo;
    int maior_combo;
    float tempo_combo;

    Arma jogador;
    Arma inimigo;
    TipoArma arma_selecionada;

    Projetil projeteis[MAX_PROJETEIS];
    Particula particulas[MAX_PARTICULAS];
    Obstaculo obstaculos[MAX_OBSTACULOS];
    Powerup powerups[MAX_POWERUPS];

    int quantidade_obstaculos;
    int quantidade_powerups;

    MarcadorColisao marcadores_cenario[MAX_MARCADORES_CENARIO];
    int quantidade_marcadores_cenario;

    Rectangle arena;
    Rectangle parede_esquerda;
    Rectangle parede_direita;
    Rectangle teto;
    Rectangle chao;

    float tempo_estado;
    bool toque_anterior;
} Jogo;

#endif
