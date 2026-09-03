#include "fases.h"

static Rectangle ret(float x, float y, float largura, float altura)
{
    return (Rectangle){x, y, largura, altura};
}

DefinicaoFase fase_obter(int indice)
{
    DefinicaoFase fase = {0};

    switch (indice)
    {
        case 0:
            fase.nome = "PRIMEIRO RECUO";
            fase.arma_inimigo = ARMA_PISTOLA;
            fase.posicao_jogador = (Vector2){145.0f, 285.0f};
            fase.posicao_inimigo = (Vector2){395.0f, 285.0f};
            fase.angulo_jogador = -90.0f;
            fase.angulo_inimigo = 90.0f;
            fase.vida_jogador = 5;
            fase.vida_inimigo = 4;
            fase.quantidade_obstaculos = 1;
            fase.obstaculos[0] = ret(205.0f, 490.0f, 130.0f, 18.0f);
            break;

        case 1:
            fase.nome = "DUPLICAÇÃO";
            fase.arma_inimigo = ARMA_PISTOLA;
            fase.posicao_jogador = (Vector2){145.0f, 260.0f};
            fase.posicao_inimigo = (Vector2){395.0f, 260.0f};
            fase.angulo_jogador = -105.0f;
            fase.angulo_inimigo = 74.0f;
            fase.vida_jogador = 5;
            fase.vida_inimigo = 5;
            fase.quantidade_obstaculos = 2;
            fase.obstaculos[0] = ret(80.0f, 485.0f, 170.0f, 16.0f);
            fase.obstaculos[1] = ret(310.0f, 590.0f, 150.0f, 16.0f);
            fase.quantidade_powerups = 1;
            fase.tipos_powerups[0] = POWERUP_DUPLO;
            fase.areas_powerups[0] = ret(45.0f, 785.0f, 58.0f, 58.0f);
            break;

        case 2:
            fase.nome = "TRIPLO CAOS";
            fase.arma_inimigo = ARMA_REVOLVER;
            fase.posicao_jogador = (Vector2){150.0f, 300.0f};
            fase.posicao_inimigo = (Vector2){390.0f, 250.0f};
            fase.angulo_jogador = -68.0f;
            fase.angulo_inimigo = 110.0f;
            fase.vida_jogador = 6;
            fase.vida_inimigo = 6;
            fase.quantidade_obstaculos = 2;
            fase.obstaculos[0] = ret(160.0f, 390.0f, 220.0f, 15.0f);
            fase.obstaculos[1] = ret(160.0f, 650.0f, 220.0f, 15.0f);
            fase.quantidade_powerups = 2;
            fase.tipos_powerups[0] = POWERUP_TRIPLO;
            fase.areas_powerups[0] = ret(45.0f, 520.0f, 58.0f, 58.0f);
            fase.tipos_powerups[1] = POWERUP_RICOCHETE;
            fase.areas_powerups[1] = ret(438.0f, 520.0f, 58.0f, 58.0f);
            break;

        case 3:
            fase.nome = "CAVEIRA";
            fase.arma_inimigo = ARMA_REVOLVER;
            fase.posicao_jogador = (Vector2){145.0f, 270.0f};
            fase.posicao_inimigo = (Vector2){395.0f, 315.0f};
            fase.angulo_jogador = -112.0f;
            fase.angulo_inimigo = 66.0f;
            fase.vida_jogador = 6;
            fase.vida_inimigo = 7;
            fase.quantidade_obstaculos = 3;
            fase.obstaculos[0] = ret(95.0f, 420.0f, 120.0f, 16.0f);
            fase.obstaculos[1] = ret(325.0f, 420.0f, 120.0f, 16.0f);
            fase.obstaculos[2] = ret(205.0f, 600.0f, 130.0f, 16.0f);
            fase.quantidade_powerups = 2;
            fase.tipos_powerups[0] = POWERUP_CAVEIRA;
            fase.areas_powerups[0] = ret(45.0f, 795.0f, 62.0f, 62.0f);
            fase.tipos_powerups[1] = POWERUP_DUPLO;
            fase.areas_powerups[1] = ret(432.0f, 300.0f, 56.0f, 56.0f);
            break;

        case 4:
            fase.nome = "ESPINGARDA";
            fase.arma_inimigo = ARMA_ESPINGARDA;
            fase.posicao_jogador = (Vector2){135.0f, 245.0f};
            fase.posicao_inimigo = (Vector2){405.0f, 245.0f};
            fase.angulo_jogador = -72.0f;
            fase.angulo_inimigo = 108.0f;
            fase.vida_jogador = 7;
            fase.vida_inimigo = 8;
            fase.quantidade_obstaculos = 4;
            fase.obstaculos[0] = ret(50.0f, 370.0f, 140.0f, 14.0f);
            fase.obstaculos[1] = ret(350.0f, 370.0f, 140.0f, 14.0f);
            fase.obstaculos[2] = ret(50.0f, 655.0f, 140.0f, 14.0f);
            fase.obstaculos[3] = ret(350.0f, 655.0f, 140.0f, 14.0f);
            fase.quantidade_powerups = 2;
            fase.tipos_powerups[0] = POWERUP_VELOCIDADE;
            fase.areas_powerups[0] = ret(242.0f, 490.0f, 58.0f, 58.0f);
            fase.tipos_powerups[1] = POWERUP_RICOCHETE;
            fase.areas_powerups[1] = ret(242.0f, 720.0f, 58.0f, 58.0f);
            break;

        default:
            fase.nome = "BULLET STORM";
            fase.arma_inimigo = ARMA_REVOLVER;
            fase.posicao_jogador = (Vector2){145.0f, 280.0f};
            fase.posicao_inimigo = (Vector2){395.0f, 280.0f};
            fase.angulo_jogador = -90.0f;
            fase.angulo_inimigo = 90.0f;
            fase.vida_jogador = 8;
            fase.vida_inimigo = 10;
            fase.quantidade_obstaculos = 4;
            fase.obstaculos[0] = ret(75.0f, 390.0f, 150.0f, 14.0f);
            fase.obstaculos[1] = ret(315.0f, 390.0f, 150.0f, 14.0f);
            fase.obstaculos[2] = ret(75.0f, 650.0f, 150.0f, 14.0f);
            fase.obstaculos[3] = ret(315.0f, 650.0f, 150.0f, 14.0f);
            fase.quantidade_powerups = 4;
            fase.tipos_powerups[0] = POWERUP_DUPLO;
            fase.areas_powerups[0] = ret(48.0f, 510.0f, 52.0f, 52.0f);
            fase.tipos_powerups[1] = POWERUP_TRIPLO;
            fase.areas_powerups[1] = ret(440.0f, 510.0f, 52.0f, 52.0f);
            fase.tipos_powerups[2] = POWERUP_CAVEIRA;
            fase.areas_powerups[2] = ret(244.0f, 530.0f, 58.0f, 58.0f);
            fase.tipos_powerups[3] = POWERUP_RICOCHETE;
            fase.areas_powerups[3] = ret(244.0f, 735.0f, 58.0f, 58.0f);
            break;
    }

    return fase;
}
